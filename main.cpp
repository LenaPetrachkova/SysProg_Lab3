#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <cstring>
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
    regex float_regex("[-+]?[0-9]*\\.[0-9]+([eE][-+]?[0-9]+)?");
    return regex_match(token, float_regex);
}

bool isDecimal(const string& token) {
    regex decimal_regex("[-+]?\\d+");
    return regex_match(token, decimal_regex);
}

bool isIdentifier(const string& token) {
    if (token.empty()) {
        return false;
    }

    if (!((token[0] >= 'A' && token[0] <= 'Z') || (token[0] >= 'a' && token[0] <= 'z') || token[0] == '_')) {
        return false;
    }

    for (size_t i = 1; i < token.size(); ++i) {
        if (!((token[i] >= 'A' && token[i] <= 'Z') || (token[i] >= 'a' && token[i] <= 'z') || (token[i] >= '0' && token[i] <= '9') || token[i] == '_')) {
            return false;
        }
    }

    return true;
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

vector<string> splitDots(const string& token) {
    vector<string> tokens;
    size_t start = 0, end = 0;

    while ((end = token.find('.', start)) != string::npos) {
        tokens.push_back(token.substr(start, end - start));
        tokens.emplace_back(".");
        start = end + 1;
    }

    tokens.push_back(token.substr(start));

    return tokens;
}

TokenList analyzeToken(const string& curr_token) {
    TokenList result;

    if (isSeparator(curr_token[0])) {
        result.emplace_back(curr_token, TT_SEPARATOR);
    } else if (isOperator(curr_token)) {
        result.emplace_back(curr_token, TT_OPERATOR);
    } else if (isHexadecimal(curr_token) || isFloatingPoint(curr_token) ||
               isDecimal(curr_token) || isStringLiteral(curr_token) ||
               isCharacterLiteral(curr_token)) {
        result.emplace_back(curr_token, TT_LITERAL);
               } else if (isKeyword(curr_token)) {
                   result.emplace_back(curr_token, TT_KEYWORD);
               } else if (isIdentifier(curr_token)) {
                   result.emplace_back(curr_token, TT_IDENTIFIER);
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

        if (sym == '/' && stringBuffer.peek() == '*') {
            continue;
        }

        if (sym == '/' && stringBuffer.peek() == '/') {
            continue;
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

        if (isalnum(sym) || sym == '_' || sym == '.') {
            curr_token += sym;
            bool started_with_digit = isdigit(sym);

            while (!stringBuffer.eof()) {
                char peek = static_cast<char>(stringBuffer.peek());
                if (!isalnum(peek) && peek != '_' && peek != '.') {
                    break;
                }
                ch = stringBuffer.get();
                if (ch == EOF) break;
                curr_token += static_cast<char>(ch);
            }

            if (started_with_digit || curr_token[0] == '.') {
                if (isFloatingPoint(curr_token)) {
                    result.emplace_back(curr_token, TT_LITERAL);
                    continue;
                }
            }

            string temp;
            for (char c : curr_token) {
                if (c == '.') {
                    if (!temp.empty()) {
                        TokenList part_result = analyzeToken(temp);
                        result.insert(result.end(), part_result.begin(), part_result.end());
                        temp.clear();
                    }
                    result.emplace_back(".", TT_SEPARATOR);
                } else {
                    temp += c;
                }
            }
            if (!temp.empty()) {
                TokenList part_result = analyzeToken(temp);
                result.insert(result.end(), part_result.begin(), part_result.end());
            }
            continue;
        }
        if (ispunct(sym)) {
            curr_token += sym;
        }

        if (!curr_token.empty()) {
            TokenList part_result = analyzeToken(curr_token);
            result.insert(result.end(), part_result.begin(), part_result.end());
        }
    }

    return result;
}

int main() {
    // string code = "let x = 10.44;\n"
    //               "/* This is a comment */\n"
    //               "x += 5;\n"
    //               "if (x > 15) console.log(\"x is greater\");\n";

    // string code = "let color = 0xFFEE88;\n"
    //             "let text = \"This is a string\";\n";

    string code = "let a = 42;\n"
                    "const pi = 3.14;\n"
                    "var hexValue = 0x1A3F;\n"
                    "\n"
                    "function greet(name) {\n"
                    "    console.log(\"Hello, \" + name);\n"
                    "}\n"
                    "\n"
                    "greet(\"World\");\n";


    TokenList token_list = analyzeCode(code);

    for (const auto& token : token_list) {
        cout << "<" << token.first << ", " << token.second << ">" << endl;
    }

    return 0;
}
