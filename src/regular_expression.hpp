#ifndef REGULAR_EXPRESSION_HPP
#define REGULAR_EXPRESSION_HPP

#include <unordered_map>
#include <variant>
#include <memory>

#include "../lib/parser.hpp"

enum RegularExpressionType
{
    EMPTY,
    CHARACTER,
    CONCAT,
    PLUS,
    STAR
};

class RegularExpression
{
    private:
        RegularExpressionType type;

        typedef std::variant<
            std::monostate,
            char,
            std::pair<std::shared_ptr<RegularExpression>, std::shared_ptr<RegularExpression>>,
            std::shared_ptr<RegularExpression>
        > Value;
        Value value;

        RegularExpression(RegularExpressionType type, Value value): type(type), value(value) {};

    public:
        RegularExpression() = default;

        static RegularExpression empty();
        static RegularExpression character(char c);
        static RegularExpression concat(RegularExpression re1, RegularExpression re2);
        static RegularExpression plus(RegularExpression re1, RegularExpression re2);
        static RegularExpression star(RegularExpression re);

        static RegularExpression fromToken(Token token);

        static RegularExpression fromExpressionString(std::string expressionStr);

        RegularExpressionType getType();

        char getCharacterExpression();
        std::pair<std::shared_ptr<RegularExpression>, std::shared_ptr<RegularExpression>> getConcatExpression();
        std::pair<std::shared_ptr<RegularExpression>, std::shared_ptr<RegularExpression>> getPlusExpression();
        std::shared_ptr<RegularExpression> getStarExpression();

        std::string toString();
};

#endif