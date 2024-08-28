#include <string>
#include <regex>
#include <iostream>

class AntMatcher
{
public:
    bool match(std::string &pattern, std::string &path)
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
                std::string paramRegex = "[^/]+";

                // 检查是否有正则部分
                int colonPos = param.find(':');
                if (colonPos != std::string::npos)
                {
                    paramRegex = param.substr(colonPos + 1);
                }

                // 使用正则表达式匹配路径段
                std::regex reg(paramRegex);
                if (!std::regex_match(pathSegment, reg))
                {
                    return false; // 匹配失败
                }
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

int main()
{
    AntMatcher matcher;
    // std::string pattern = "/api/{version}/{username:[a-z]+}";
    // std::string path1 = "/api/v1/hello";
    // std::string path2 = "/api/v1/123";

    std::string pattern = "/test/{hi}/hello/{ok}";
    std::string path1 = "/test/10/hello/do";
    std::string path2 = "/api/v1/123";

    bool result1 = matcher.match(pattern, path1); // 返回 true
    bool result2 = matcher.match(pattern, path2); // 返回 false

    std::cout << "Result1: " << result1 << std::endl; // 应该输出 1 (true)
    std::cout << "Result2: " << result2 << std::endl; // 应该输出 0 (false)

    return 0;
}
