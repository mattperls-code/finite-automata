#include <fstream>
#include <cstdlib>
#include <filesystem>

#include "regular_expression.hpp"

RegularExpression RegularExpression::empty()
{
    return RegularExpression(RegularExpressionType::EMPTY, {});
};

RegularExpression RegularExpression::character(char c)
{
    return RegularExpression(RegularExpressionType::CHARACTER, c);
};

RegularExpression RegularExpression::concat(RegularExpression re1, RegularExpression re2)
{
    if (re1.type == EMPTY) return re2;
    if (re2.type == EMPTY) return re1;

    return RegularExpression(RegularExpressionType::CONCAT, std::pair<std::shared_ptr<RegularExpression>, std::shared_ptr<RegularExpression>> {
        std::make_shared<RegularExpression>(re1),
        std::make_shared<RegularExpression>(re2)
    });
};

RegularExpression RegularExpression::plus(RegularExpression re1, RegularExpression re2)
{
    return RegularExpression(RegularExpressionType::PLUS, std::pair<std::shared_ptr<RegularExpression>, std::shared_ptr<RegularExpression>> {
        std::make_shared<RegularExpression>(re1),
        std::make_shared<RegularExpression>(re2)
    });
};

RegularExpression RegularExpression::star(RegularExpression re)
{
    return RegularExpression(RegularExpressionType::STAR, std::make_shared<RegularExpression>(re));
};

RegularExpression RegularExpression::fromToken(Token token)
{
    if (token.id == "CHAR") return RegularExpression::character(token.getStringLiteralContent()[0]);

    std::vector<RegularExpression> nestedExpressions;
    for (auto nestedToken : token.getNestingContent()) nestedExpressions.push_back(RegularExpression::fromToken(nestedToken));

    if (token.id == "CONCAT") return RegularExpression::concat(nestedExpressions[0], nestedExpressions[1]);

    if (token.id == "PLUS") return RegularExpression::plus(nestedExpressions[0], nestedExpressions[1]);
    
    if (token.id == "STAR") return RegularExpression::star(nestedExpressions[0]);

    return RegularExpression::empty();
};

RegularExpression RegularExpression::fromExpressionString(std::string expressionStr)
{
    // uses parser lib to construct expression tree from a re string, then pass to fromToken to build actual re

    // combinators are a bit tricky here since it has to avoid left recursion
    // to do this we use atomic expressions which dont self recurse and then build recursive operational expressions layer by layer

    ParserCombinator expression;
    
    auto whitespace = repetition(satisfy(is(' ')));

    auto characterExpression = satisfy("CHAR", isalnum);

    // λ
    auto lambdaExpression = sequence("EMPTY", {
        satisfy(is((char) 206)),
        satisfy(is((char) 187))
    });

    auto groupExpression = sequence({
        satisfy(is('(')),
        proxyParserCombinator(&expression),
        satisfy(is(')'))
    });

    auto atom = choice({
        characterExpression,
        lambdaExpression,
        groupExpression
    });

    auto starExpression = atom.followedBy("STAR", satisfy(is('*')));

    auto atomOrStarExpression = choice({
        starExpression,
        atom
    });

    ParserCombinator concatExpression = sequence("CONCAT", {
        atomOrStarExpression,
        whitespace,
        choice({
            proxyParserCombinator(&concatExpression),
            atomOrStarExpression
        })
    });

    ParserCombinator plusExpression = sequence("PLUS", {
        choice({
            concatExpression,
            atomOrStarExpression
        }),
        satisfy(is('+')).surroundedBy(whitespace),
        choice({
            proxyParserCombinator(&plusExpression),
            concatExpression,
            atomOrStarExpression
        })
    });

    expression = choice({
        plusExpression,
        concatExpression,
        starExpression,
        atom
    }).surroundedBy(whitespace);

    auto grammar = strictlySequence({
        expression,
        satisfy(is('\0'))
    });
    
    auto parseResult = parse(expressionStr + '\0', grammar);

    if (getResultType(parseResult) == PARSER_FAILURE) throw std::runtime_error("RegularExpression fromExpressionString: " + getParserFailureFromResult(parseResult).toString());

    return RegularExpression::fromToken(getTokenFromResult(parseResult).getNestingContent()[0]);
};

RegularExpressionType RegularExpression::getType()
{
    return this->type;
};

char RegularExpression::getCharacterExpression()
{
    return std::get<char>(this->value);
};

std::pair<std::shared_ptr<RegularExpression>, std::shared_ptr<RegularExpression>> RegularExpression::getConcatExpression()
{
    return std::get<std::pair<std::shared_ptr<RegularExpression>, std::shared_ptr<RegularExpression>>>(this->value);
};

std::pair<std::shared_ptr<RegularExpression>, std::shared_ptr<RegularExpression>> RegularExpression::getPlusExpression()
{
    return std::get<std::pair<std::shared_ptr<RegularExpression>, std::shared_ptr<RegularExpression>>>(this->value);
};

std::shared_ptr<RegularExpression> RegularExpression::getStarExpression()
{
    return std::get<std::shared_ptr<RegularExpression>>(this->value);
};

std::string RegularExpression::toString()
{
    if (this->type == EMPTY) return "λ";

    if (this->type == CHARACTER) return std::string(1, std::get<char>(this->value));

    if (this->type == STAR) {
        auto operand = std::get<std::shared_ptr<RegularExpression>>(this->value);

        auto operandString = operand->toString();

        if (operand->getType() == PLUS || operand->getType() == CONCAT) operandString = "(" + operandString + ")";

        return operandString + "*";
    }

    auto operands = std::get<std::pair<std::shared_ptr<RegularExpression>, std::shared_ptr<RegularExpression>>>(this->value);

    auto leftOperandString = operands.first->toString();
    auto rightOperandString = operands.second->toString();

    if (this->type == PLUS) return leftOperandString + "+" + rightOperandString;
    
    // if either operand comes from a plus, it needs to be wrapped before concat to ensure correct distribution

    if (operands.first->getType() == PLUS) leftOperandString = "(" + leftOperandString + ")";
    if (operands.second->getType() == PLUS) rightOperandString = "(" + rightOperandString + ")";

    return leftOperandString + rightOperandString;
};

std::string RegularExpression::toLatex()
{
    std::string output;

    output += "\\documentclass{article}";

    output += "\n";

    output += "\\usepackage{amsmath}";

    output += "\n";

    output += "\\begin{document}";

    output += "\n";

    std::string raw = this->toString();
    std::string escaped;
    
    while (!raw.empty()) {
        if (raw.starts_with("λ")) {
            escaped += "\\lambda";
            
            raw.erase(raw.begin(), raw.begin() + std::string("λ").size());
        }
        else {
            char c = raw.front();

            if (c == '\\') escaped += "\\textbackslash{}";
            else if (
                c == '{' ||
                c == '}' ||
                c == '_' ||
                c == '^' ||
                c == '$' ||
                c == '&' ||
                c == '#' ||
                c == '%' ||
                c == '~'
            ) escaped += "\\" + std::string(c, 1);
            else if(c == '*') escaped += "^*";
            else escaped += c;

            raw.erase(raw.begin());
        }
    }

    output += "{\\Huge \\[ " + escaped + " \\] }";

    output += "\n";

    output += "\\end{document}";

    return output;
};

void RegularExpression::exportExpression(std::string outputDirPath, std::string outputFileName)
{
    std::filesystem::create_directories(outputDirPath);

    std::string latexOutputFilePath = outputDirPath + "/" + outputFileName + ".tex";

    std::ofstream latexOutputFile(latexOutputFilePath);

    latexOutputFile << this->toLatex();

    latexOutputFile.close();

    // pdflatex sometimes needs to be called twice
    std::string renderLaTeXCommand = 
        "pdflatex -output-directory=" + outputDirPath + " " + latexOutputFilePath + 
        " > /dev/null 2>&1 && " + 
        "pdflatex -output-directory=" + outputDirPath + " " + latexOutputFilePath + 
        " > /dev/null 2>&1 && " + 
        "rm -f " + outputDirPath + "/*.aux " + outputDirPath + "/*.log " + outputDirPath + "/*.out " + outputDirPath + "/*.toc";

    std::system(renderLaTeXCommand.c_str());
};