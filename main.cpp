#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <regex>

using namespace std;

const string TT_KEYWORD = "keyword";
const string TT_IDENTIFIER = "identifier";
const string TT_SEPARATOR = "separator";
const string TT_OPERATOR = "operator";
const string TT_LITERAL = "literal";
const string TT_COMMENT = "comment";
const string TT_ERROR = "error: unrecognized symbol";

typedef vector<pair<string, string>> TokenList;

bool isHexadecimal(const string& token) {
    regex hex_regex("0[xX][0-9a-fA-F]+");
    return regex_match(token, hex_regex);
}

bool isFloatingPoint(const string& token) {
    regex float_regex("[+-]?\\d+\\.\\d+");
    return regex_match(token, float_regex);
}

bool isDecimal(const string& token) {
    regex decimal_regex("[+-]?\\d+");
    return regex_match(token, decimal_regex);
}

bool isIdentifier(const string& token) {
    if (token.empty()) return false;
    if (!isalpha(token[0]) && token[0] != '_') return false;
    return all_of(token.begin() + 1, token.end(), [](char ch) {
        return isalnum(ch) || ch == '_';
    });
}

bool isStringLiteral(const string& token) {
    return token.size() >= 2 && token.front() == '"' && token.back() == '"';
}

bool isCharacterLiteral(const string& token) {
    regex char_literal_regex(R"('(?:\\.|[^\\'])')");
    return regex_match(token, char_literal_regex);
}

bool isSeparator(char c) {
    return c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']' || c == ';' || c == ',' || c == '.';
}

bool isKeyword(const string& token) {
    vector<string> keywords = {
        "abstract", "arguments", "await", "boolean", "break", "byte", "case", "catch", "char", "class", "const", "continue",
        "debugger", "default", "delete", "do", "double", "else", "enum", "eval", "export", "extends", "false", "final",
        "finally", "float", "for", "function", "goto", "if", "implements", "import", "in", "instanceof", "int",
        "interface", "let", "long", "native", "new", "null", "package", "private", "protected", "public", "return",
        "short", "static", "super", "switch", "synchronized", "this", "throw", "throws", "transient", "true", "try",
        "typeof", "var", "void", "volatile", "while", "with", "yield"
    };
    return find(keywords.begin(), keywords.end(), token) != keywords.end();
}

bool isOperator(const string& token) {
    vector<string> operators = {
        "==", "!=", "<=", ">=", "&&", "||", "!", "++", "--", "+=", "-=", "*=", "/=", "%=",
        "+", "-", "*", "/", "%", "&", "|", "^", "~", "<<", ">>", "=", "<", ">"
    };
    return find(operators.begin(), operators.end(), token) != operators.end();
}

TokenList analyzeToken(const string& curr_token) {
    TokenList result;

    if (isHexadecimal(curr_token)) {
        result.emplace_back(curr_token, TT_LITERAL);
    } else if (isFloatingPoint(curr_token)) {
        result.emplace_back(curr_token, TT_LITERAL);
    } else if (isKeyword(curr_token)) {
        result.emplace_back(curr_token, TT_KEYWORD);
    } else if (isIdentifier(curr_token)) {
        result.emplace_back(curr_token, TT_IDENTIFIER);
    } else if (isDecimal(curr_token)) {
        result.emplace_back(curr_token, TT_LITERAL);
    } else {
        result.emplace_back(curr_token, TT_ERROR);
    }

    return result;
}


TokenList analyzeCode(const string& code) {
    TokenList result;
    istringstream stringBuffer(code);
    bool in_string_literal = false;

    while (!stringBuffer.eof()) {
        int ch = stringBuffer.get();
        if (ch == EOF) break;

        char sym = static_cast<char>(ch);
        string curr_token;

        while (isspace(sym) && !stringBuffer.eof()) {
            ch = stringBuffer.get();
            if (ch == EOF) break;
            sym = static_cast<char>(ch);
        }

        if (stringBuffer.eof()) {
            break;
        }

        if (sym == '"' || in_string_literal) {
            in_string_literal = true;
            curr_token = "\"";
            while (in_string_literal && !stringBuffer.eof()) {
                ch = stringBuffer.get();
                if (ch == EOF) break;
                sym = static_cast<char>(ch);
                curr_token += sym;

                if (sym == '\\') {
                    ch = stringBuffer.get();
                    if (ch == EOF) break;
                    curr_token += static_cast<char>(ch);
                } else if (sym == '"') {
                    in_string_literal = false;
                }
            }
            result.emplace_back(curr_token, TT_LITERAL);
            continue;
        }

        if (isalnum(sym) || sym == '_') {
            curr_token += sym;

            while (!stringBuffer.eof() && (isalnum(stringBuffer.peek()) || stringBuffer.peek() == '_' || stringBuffer.peek() == '.')) {
                ch = stringBuffer.get();
                if (ch == EOF) break;
                curr_token += static_cast<char>(ch);
            }

            if (count(curr_token.begin(), curr_token.end(), '.') > 1) {
                result.emplace_back(curr_token, TT_ERROR);
            } else {
                TokenList tokenized = analyzeToken(curr_token);
                result.insert(result.end(), tokenized.begin(), tokenized.end());
            }
            continue;
        }

        if (ispunct(sym)) {
            string two_char_token(1, sym);
            if (!stringBuffer.eof() && ispunct(stringBuffer.peek())) {
                two_char_token += static_cast<char>(stringBuffer.get());
                if (isOperator(two_char_token)) {
                    result.emplace_back(two_char_token, TT_OPERATOR);
                    continue;
                } else {
                    stringBuffer.putback(two_char_token[1]);
                    two_char_token = sym;
                }
            }
            result.emplace_back(two_char_token, isOperator(two_char_token) ? TT_OPERATOR : TT_SEPARATOR);
        }
    }

    return result;
}

int main() {
    string code = "5for 5for5 for5 xx====8.9.0--<<-====\n"
                  "let a = 42;\n"
                  "const pi = 3.14;\n"
                  "var hexValue = 0x1A3F;\n"
                  "function greet(name) {\n"
                  "greet(\"World\");\n";

    TokenList tokens = analyzeCode(code);

    for (const auto& token : tokens) {
        cout << token.first << " - " << token.second << endl;
    }

    return 0;
}
