#pragma once
#include "ntshengn_defines.h"
#include <string>
#include <fstream>
#include <vector>
#include <forward_list>
#include <unordered_map>
#include <variant>

#if defined(NTSHENGN_DEBUG)
#define NTSHENGN_JSON_INFO(message) \
	do { \
		std::cerr << "\33[1m\33[37mJSON \33[34mINFO\33[39m\33[0m: " << message << std::endl; \
	} while(0)
#else
#define NTSHENGN_JSON_INFO(message) \
	do { \
	} while(0)
#endif

#if defined(NTSHENGN_DEBUG)
#define NTSHENGN_JSON_WARNING(message) \
	do { \
		std::cerr << "\33[1m\33[37mJSON \33[93mWARNING\33[39m\33[0m: " << message << std::endl; \
	} while(0)
#else
#define NTSHENGN_JSON_WARNING(message) \
	do { \
	} while(0)
#endif

#if defined(NTSHENGN_DEBUG)
#define NTSHENGN_JSON_ERROR(message, code) \
	do { \
		std::cerr << "\33[1m\33[37mJSON \33[31mERROR\33[39m\33[0m: " << message << " (" << #code << ")" << std::endl; \
		exit(1); \
	} while(0)
#else
#define NTSHENGN_JSON_ERROR(message, code) \
	do { \
		exit(1); \
	} while(0)
#endif

namespace NtshEngn {

	class JSON {
	public:
		enum class Type {
			Object,
			Number,
			String,
			Array,
			Boolean,
			Null
		};

		class Node {
		public:
			// Construct JSON::Type::Object
			Node(const std::unordered_map<std::string, Node*> children) : m_type(Type::Object), m_value(children) {}

			// Construct JSON::Type::Number
			Node(float number) : m_type(Type::Number), m_value(number) {}

			// Construct JSON::Type::String
			Node(const std::string& string) : m_type(Type::String), m_value(string) {}

			// Construct JSON::Type::Array
			Node(const std::vector<Node*>& array) : m_type(Type::Array), m_value(array) {}

			// Construct JSON::Type::Boolean
			Node(bool boolean) : m_type(Type::Boolean), m_value(boolean) {}

			// Construct JSON::Type::Null
			Node() : m_type(Type::Null), m_value() {}

			Type getType() const {
				return m_type;
			}

			bool contains(const std::string& childName) const {
				NTSHENGN_ASSERT(m_type == Type::Object);

				const std::unordered_map<std::string, Node*>& val = std::get<std::unordered_map<std::string, Node*>>(m_value);

				return (val.find(childName) != val.end());
			}

			size_t size() const {
				NTSHENGN_ASSERT((m_type == Type::Object) || (m_type == Type::Array));

				size_t size = 0;
				if (m_type == Type::Object) {
					size = std::get<std::unordered_map<std::string, Node*>>(m_value).size();
				}
				else if (m_type == Type::Array) {
					size = std::get<std::vector<Node*>>(m_value).size();
				}

				return size;
			}

			// Access JSON::Type::Object
			Node& operator[](const std::string& childName) {
				NTSHENGN_ASSERT(m_type == Type::Object);

				const std::unordered_map<std::string, Node*>& val = std::get<std::unordered_map<std::string, Node*>>(m_value);
				NTSHENGN_ASSERT(val.find(childName) != val.end());

				return *val.at(childName);
			}

			const Node& operator[](const std::string& childName) const {
				NTSHENGN_ASSERT(m_type == Type::Object);

				const std::unordered_map<std::string, Node*>& val = std::get<std::unordered_map<std::string, Node*>>(m_value);
				NTSHENGN_ASSERT(val.find(childName) != val.end());

				return *val.at(childName);
			}

			const std::vector<std::string> getKeys() const {
				NTSHENGN_ASSERT(m_type == Type::Object);

				std::vector<std::string> keys;

				const std::unordered_map<std::string, Node*>& val = std::get<std::unordered_map<std::string, Node*>>(m_value);
				for (const auto& keyNodePair : val) {
					keys.push_back(keyNodePair.first);
				}

				return keys;
			}

			// Access JSON::Type::Number
			float getNumber() const {
				NTSHENGN_ASSERT(m_type == Type::Number);

				const float val = std::get<float>(m_value);

				return val;
			}

			// Access JSON::Type::String
			std::string getString() const {
				NTSHENGN_ASSERT(m_type == Type::String);

				const std::string& val = std::get<std::string>(m_value);

				return val;
			}

			// Access JSON::Type::Array
			Node& operator[](const size_t element) {
				NTSHENGN_ASSERT(m_type == Type::Array);

				const std::vector<Node*>& val = std::get<std::vector<Node*>>(m_value);
				NTSHENGN_ASSERT(element < val.size());

				return *val.at(element);
			}

			const Node& operator[](const size_t element) const {
				NTSHENGN_ASSERT(m_type == Type::Array);

				const std::vector<Node*>& val = std::get<std::vector<Node*>>(m_value);
				NTSHENGN_ASSERT(element < val.size());

				return *val.at(element);
			}

			// Access JSON::Type::Boolean
			bool getBoolean() const {
				NTSHENGN_ASSERT(m_type == Type::Boolean);

				const bool val = std::get<bool>(m_value);

				return val;
			}

			// Add object to JSON::Type::Object
			void addObject(const std::string& childName, Node* childNode) {
				NTSHENGN_ASSERT((m_type == Type::Object) || (m_type == Type::Null));

				std::unordered_map<std::string, Node*>& val = std::get<std::unordered_map<std::string, Node*>>(m_value);
				NTSHENGN_ASSERT(val.find(childName) == val.end());

				m_type = Type::Object;
				val[childName] = childNode;
			}

			// Set number to JSON::Type::Number
			void setNumber(float number) {
				NTSHENGN_ASSERT((m_type == Type::Number) || (m_type == Type::Null));

				float& val = std::get<float>(m_value);

				m_type = Type::Number;
				val = number;
			}

			// Set string to JSON::Type::String
			void setString(const std::string& string) {
				NTSHENGN_ASSERT((m_type == Type::String) || (m_type == Type::Null));

				std::string& val = std::get<std::string>(m_value);

				m_type = Type::String;
				val = string;
			}

			// Add object to JSON::Type::Array
			void addObject(Node* element) {
				NTSHENGN_ASSERT((m_type == Type::Array) || (m_type == Type::Null));

				std::vector<Node*>& val = std::get<std::vector<Node*>>(m_value);

				m_type = Type::Array;
				val.push_back(element);
			}

			// Set boolean to JSON::Type::Boolean
			void setBoolean(bool boolean) {
				NTSHENGN_ASSERT((m_type == Type::Boolean) || (m_type == Type::Null));
				
				bool& val = std::get<bool>(m_value);

				m_type = Type::Boolean;
				val = boolean;
			}

		private:
			Type m_type;
			std::variant<std::unordered_map<std::string, Node*>, float, std::string, std::vector<Node*>, bool> m_value;
		};

	private:
		enum class TokenType {
			CurlyBracketOpen,
			CurlyBracketClose,
			Colon,
			Comma,
			String,
			Number,
			ArrayBracketOpen,
			ArrayBracketClose,
			Boolean,
			Null,
			EndOfFile
		};

		struct Token {
			TokenType type;
			std::string value = "";
		};

		class Lexer {
		public:
			~Lexer() {
				if (m_file.is_open()) {
					m_file.close();
				}
			}

			void open(const std::string& filePath) {
				m_file.open(filePath, std::fstream::in | std::fstream::binary);
				if (!m_file.is_open()) {
					NTSHENGN_JSON_ERROR("Cannot open JSON file \"" + filePath + "\".", Result::JSONError);
				}
			}

			char nextCharacter() {
				char c = ' ';
				while ((c == ' ') || (c == '\n') || (c == '\r') || (c == '\t')) {
					m_file.get(c);

					if (((c == ' ') || (c == '\n') || (c == '\r') || (c == '\t')) && m_file.eof()) {
						return '\0';
					}
				}

				return c;
			}

			Token getNextToken() {
				Token token;

				char c;

				if (m_file.eof()) {
					NTSHENGN_JSON_ERROR("Reached end-of-file early.", Result::JSONError);
				}

				m_previousPosition = m_file.tellg();
				c = nextCharacter();
				if (c == '"') {
					token.type = TokenType::String;
					token.value = "";

					m_file.get(c);
					while (c != '\"') {
						token.value += c;

						m_file.get(c);
					}
				}
				else if (c == '{') {
					token.type = TokenType::CurlyBracketOpen;
				}
				else if (c == '}') {
					token.type = TokenType::CurlyBracketClose;
				}
				else if (c == '[') {
					token.type = TokenType::ArrayBracketOpen;
				}
				else if (c == ']') {
					token.type = TokenType::ArrayBracketClose;
				}
				else if (c == ':') {
					token.type = TokenType::Colon;
				}
				else if (c == ',') {
					token.type = TokenType::Comma;
				}
				else if (c == '-' || ((c >= '0') && (c <= '9'))) {
					token.type = TokenType::Number;
					token.value += c;

					size_t previousCharPosition;
					while ((c == '-') || (c == '.') || (c == 'e') || ((c >= '0') && (c <= '9'))) {
						previousCharPosition = m_file.tellg();
						m_file.get(c);

						if (m_file.eof()) {
							break;
						}

						if ((c == '-') || (c == '.') || (c == 'e') || ((c >= '0') && (c <= '9'))) {
							token.value += c;
						}
						else {
							m_file.seekg(previousCharPosition);
							break;
						}
					}
				}
				else if (c == 't') {
					token.type = TokenType::Boolean;
					token.value = "t";

					m_file.get(c);
					token.value += c;

					if (c == 'r') {
						m_file.get(c);
						token.value += c;

						if (c == 'u') {
							m_file.get(c);
							token.value += c;

							if (c != 'e') {
								NTSHENGN_JSON_ERROR("\"" + token.value + "\" token is invalid.", Result::JSONError);
							}
						}
						else {
							NTSHENGN_JSON_ERROR("\"" + token.value + "\" token is invalid.", Result::JSONError);
						}
					}
					else {
						NTSHENGN_JSON_ERROR("\"" + token.value + "\" token is invalid.", Result::JSONError);
					}
				}
				else if (c == 'f') {
					token.type = TokenType::Boolean;
					token.value = "f";

					m_file.get(c);
					token.value += c;

					if (c == 'a') {
						m_file.get(c);
						token.value += c;

						if (c == 'l') {
							m_file.get(c);
							token.value += c;

							if (c == 's') {
								m_file.get(c);
								token.value += c;

								if (c != 'e') {
									NTSHENGN_JSON_ERROR("\"" + token.value + "\" token is invalid.", Result::JSONError);
								}
							}
							else {
								NTSHENGN_JSON_ERROR("\"" + token.value + "\" token is invalid.", Result::JSONError);
							}
						}
						else {
							NTSHENGN_JSON_ERROR("\"" + token.value + "\" token is invalid.", Result::JSONError);
						}
					}
					else {
						NTSHENGN_JSON_ERROR("\"" + token.value + "\" token is invalid.", Result::JSONError);
					}
				}
				else if (c == 'n') {
					token.type = TokenType::Null;
					std::string nullValue = "n";

					m_file.get(c);
					nullValue += c;
					if (c == 'u') {
						m_file.get(c);
						nullValue += c;

						if (c == 'l') {
							m_file.get(c);
							nullValue += c;

							if (c != 'l') {
								NTSHENGN_JSON_ERROR("\"" + nullValue + "\" token is invalid.", Result::JSONError);
							}
						}
						else {
							NTSHENGN_JSON_ERROR("\"" + nullValue + "\" token is invalid.", Result::JSONError);
						}
					}
					else {
						NTSHENGN_JSON_ERROR("\"" + nullValue + "\" token is invalid.", Result::JSONError);
					}
				}
				else if (c == '\0') {
					token.type = TokenType::EndOfFile;
				}
				else {
					NTSHENGN_JSON_ERROR("Reached an unknown token (\"" + std::string() + c + "\").", Result::JSONError);
				}

				return token;
			}

			bool endOfFile() {
				return m_file.eof();
			}

		private:
			std::fstream m_file;
			size_t m_previousPosition;
		};

		class Parser {
		public:
			Node parse(const std::string& filePath) {
				m_lexer.open(filePath);
				
				Node root;
				bool rootInitialized = false;

				while (!m_lexer.endOfFile()) {
					Token token = m_lexer.getNextToken();
					Node node;
					switch (token.type) {
					case TokenType::CurlyBracketOpen: {
						node = parseObject();
						break;
					}

					case TokenType::String: {
						node = Node(token.value);
						break;
					}

					case TokenType::Number: {
						node = Node(std::stof(token.value));
						break;
					}

					case TokenType::ArrayBracketOpen: {
						node = parseArray();
						break;
					}

					case TokenType::Boolean: {
						node = Node(token.value == "true");
						break;
					}

					default:
						break;
					}

					if (!rootInitialized) {
						root = node;

						rootInitialized = true;
					}
				}

				return root;
			}

		private:
			Node parseObject() {
				Node objectNode = Node(std::unordered_map<std::string, Node*>());

				bool endOfObject = false;
				while (!endOfObject) {
					// Map key
					Token keyToken = m_lexer.getNextToken();

					// Empty object
					if (keyToken.type == TokenType::CurlyBracketClose) {
						return objectNode;
					}

					// Colon
					if (m_lexer.getNextToken().type != TokenType::Colon) {
						NTSHENGN_JSON_ERROR("An object key (\"" + keyToken.value + "\") is not followed by a colon.", Result::JSONError);
					}

					Token token = m_lexer.getNextToken();
					Node node;
					switch (token.type) {
					case TokenType::CurlyBracketOpen: {
						node = parseObject();
						break;
					}

					case TokenType::String: {
						node = Node(token.value);
						break;
					}

					case TokenType::Number: {
						node = Node(std::stof(token.value));
						break;
					}

					case TokenType::ArrayBracketOpen: {
						node = parseArray();
						break;
					}

					case TokenType::Boolean: {
						node = Node(token.value == "true");
						break;
					}

					case TokenType::EndOfFile: {
						NTSHENGN_JSON_ERROR("Reached end-of-file while parsing an object.", Result::JSONError);
					}

					default:
						break;
					}

					m_nodes.push_front(node);
					objectNode.addObject(keyToken.value, &m_nodes.front());

					// Next token is either a comma or a curly bracket close
					if (m_lexer.getNextToken().type == TokenType::CurlyBracketClose) {
						endOfObject = true;
					}
				}

				return objectNode;
			}

			Node parseArray() {
				Node arrayNode = Node(std::vector<Node*>());

				bool endOfArray = false;
				while (!endOfArray) {
					Token token = m_lexer.getNextToken();

					// Empty array
					if (token.type == TokenType::ArrayBracketClose) {
						return arrayNode;
					}

					Node node;
					switch (token.type) {
					case TokenType::CurlyBracketOpen: {
						node = parseObject();
						break;
					}

					case TokenType::String: {
						node = Node(token.value);
						break;
					}

					case TokenType::Number: {
						node = Node(std::stof(token.value));
						break;
					}

					case TokenType::ArrayBracketOpen: {
						node = parseArray();
						break;
					}

					case TokenType::Boolean: {
						node = Node(token.value == "true");
						break;
					}

					case TokenType::EndOfFile: {
						NTSHENGN_JSON_ERROR("Reached end-of-file while parsing an array.", Result::JSONError);
					}

					default:
						break;
					}

					m_nodes.push_front(node);
					arrayNode.addObject(&m_nodes.front());

					// Next token is either a comma or an array bracket close
					if (m_lexer.getNextToken().type == TokenType::ArrayBracketClose) {
						endOfArray = true;
					}
				}

				return arrayNode;
			}

		private:
			Lexer m_lexer;
			
			std::forward_list<Node> m_nodes;
		};

	public:
		Node read(const std::string& filePath) {
			return m_parser.parse(filePath);
		}

	public:
		static std::string to_string(const Node& node) {
			return to_string(node, 0, false);
		}

	private:
		static std::string to_string(const Node& node, size_t indentationLevel, bool indentFirst) {
			const std::string indentation = std::string(indentationLevel, '\t');
			std::string jsonString = (indentFirst ? indentation : "");

			switch (node.getType()) {
			case Type::Object: {
				jsonString += "{\n";
				std::vector<std::string> keys = node.getKeys();
				for (const std::string& key : keys) {
					jsonString += indentation + "\t\"" + key + "\": " + to_string(node[key], indentationLevel + 1, false);
					jsonString += (key != keys.back()) ? ",\n" : "\n";
				}
				jsonString += indentation + "}";
				break;
			}

			case Type::Number: {
				jsonString += std::to_string(node.getNumber());
				break;
			}

			case Type::String: {
				jsonString += "\"" + node.getString() + "\"";
				break;
			}

			case Type::Array: {
				jsonString += "[\n";
				for (size_t i = 0; i < node.size(); i++) {
					jsonString += to_string(node[i], indentationLevel + 1, true);
					jsonString += (i < (node.size() - 1)) ? ",\n" : "\n";
				}
				jsonString += indentation + "]";
				break;
			}

			case Type::Boolean: {
				jsonString += node.getBoolean() ? "true" : "false";
				break;
			}

			case Type::Null: {
				jsonString += "null";
				break;
			}

			default:
				break;
			}

			return jsonString;
		}

	private:
		Parser m_parser;
	};

}