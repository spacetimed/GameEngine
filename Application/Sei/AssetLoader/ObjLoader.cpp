#include "OBJLoader.h"

#include <fstream>
#include <sstream>
#include <utility>

namespace Sei::AssetLoader
{
    bool LoadOBJ(const std::filesystem::path& path, MeshData& output, std::string& error)
    {
        error.clear();

        std::ifstream file(path);
        if (!file)
        {
            error = "Could not open OBJ file: " + path.string();
            return false;
        }

        MeshData loaded;
        std::string line;
        std::size_t lineNumber = 0;

        auto fail = [&](const std::string& reason)
            {
                error = "Line " + std::to_string(lineNumber) + ": " + reason;
                return false;
            };

        while (std::getline(file, line))
        {
            ++lineNumber;

            // Remove comments, including comments after data.
            line = line.substr(0, line.find('#'));

            std::istringstream stream(line);
            std::string prefix;

            if (!(stream >> prefix))
                continue; // Blank line.

            if (prefix == "v")
            {
                Vertex vertex = {};
                std::string extra;

                if (!(stream >> vertex.x >> vertex.y >> vertex.z) ||
                    (stream >> extra))
                {
                    return fail("Expected three position coordinates.");
                }

                loaded.vertices.push_back(vertex);
            }
            else if (prefix == "f")
            {
                // Only positive, position-only triangle indices for now.
                long long a, b, c;
                std::string extra;

                if (!(stream >> a >> b >> c) || (stream >> extra))
                    return fail("Expected three position-only face indices.");

                for (long long index : { a, b, c })
                {
                    if (index <= 0 ||
                        index > static_cast<long long>(loaded.vertices.size()))
                    {
                        return fail("Face references an invalid vertex.");
                    }

                    loaded.indices.push_back(
                        static_cast<std::uint32_t>(index - 1));
                }
            }
            else
            {
                return fail("Unsupported OBJ entry: " + prefix);
            }
        }

        if (file.bad())
        {
            error = "Error reading OBJ file.";
            return false;
        }

        if (loaded.vertices.empty() || loaded.indices.empty())
        {
            error = "OBJ contains no drawable geometry.";
            return false;
        }

        output = std::move(loaded);
        return true;
    }
}