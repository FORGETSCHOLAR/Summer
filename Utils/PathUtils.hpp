#include <string>
#include <regex>
#include <iostream>
#include <map>
#include <vector>

class AntMatcher
{
public:
    static bool match(const std::string &pattern, const std::string &path, std::map<std::string, std::vector<std::string>> &queryParameters)
    {
        size_t startPattern = 0;
        size_t startPath = 0;

        while (startPattern < pattern.length() && startPath < path.length())
        {
            size_t posPattern = pattern.find("/", startPattern);
            size_t posPath = path.find("/", startPath);

            std::string patternSegment = (posPattern == std::string::npos) ? pattern.substr(startPattern) : pattern.substr(startPattern, posPattern - startPattern);
            std::string pathSegment = (posPath == std::string::npos) ? path.substr(startPath) : path.substr(startPath, posPath - startPath);

            if (!patternSegment.empty() && patternSegment[0] == '{')
            {
                // 处理变量部分
                int endBrace = patternSegment.find('}');
                if (endBrace == std::string::npos)
                {
                    return false; // 模式格式错误
                }

                std::string param = patternSegment.substr(1, endBrace - 1);
                std::string paramName, paramRegex = "[^/]+";

                // 检查是否有正则部分
                int colonPos = param.find(':');
                if (colonPos != std::string::npos)
                {
                    paramName = param.substr(0, colonPos);
                    paramRegex = param.substr(colonPos + 1);
                }
                else
                {
                    paramName = param;
                }

                // 使用正则表达式匹配路径段
                std::regex reg(paramRegex);
                if (!std::regex_match(pathSegment, reg))
                {
                    return false; // 匹配失败
                }

                // 将匹配的变量和对应值存入 map
                queryParameters[paramName].push_back(pathSegment);
            }
            else
            {
                // 处理普通字符串部分
                if (patternSegment != pathSegment)
                {
                    return false; // 字符串不匹配
                }
            }

            // 更新起始位置
            startPattern = (posPattern == std::string::npos) ? pattern.length() : posPattern + 1;
            startPath = (posPath == std::string::npos) ? path.length() : posPath + 1;
        }

        // 检查两者是否都遍历完
        return startPattern >= pattern.length() && startPath >= path.length();
    }
};
