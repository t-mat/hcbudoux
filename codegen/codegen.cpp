#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS 1
#endif
#include <json.h>  // https://github.com/sheredom/json.h/blob/master/json.h
#include <stdio.h>
#include <stdlib.h>

#include <cinttypes>
#include <map>
#include <string>
#include <vector>

// [[nodiscard]] is C++17.  Expands to nothing on older standards (-std=c++11 / /std:c++14).
// __has_cpp_attribute alone is not enough: clang reports the attribute as available in C++14
// mode and then warns under -Wpedantic, so the language version is checked as well
// (MSVC reports it in _MSVC_LANG unless /Zc:__cplusplus is given).
#if (__cplusplus >= 201703L) || (defined(_MSVC_LANG) && (_MSVC_LANG >= 201703L))
#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(nodiscard)
#define HCBUDOUX_NODISCARD [[nodiscard]]
#endif
#endif
#endif
#if !defined(HCBUDOUX_NODISCARD)
#define HCBUDOUX_NODISCARD
#endif

HCBUDOUX_NODISCARD std::string readFile(const std::string &filename) {
  std::string v;
  if (FILE *fp = fopen(filename.c_str(), "rb")) {
    for (int c; (c = fgetc(fp)) != EOF;) {
      v.push_back(static_cast<char>(c));
    }
    fclose(fp);
  }
  return v;
}

namespace TextTemplate {
using Dictionary = std::map<std::string, std::string>;

HCBUDOUX_NODISCARD std::string replaceAll(const std::string &src, const Dictionary &dictionary) {
  std::string tempText = src;
  for (const auto &kv : dictionary) {
    const std::string &replaceWord = kv.first;
    const std::string &replaceBy = kv.second;
    for (size_t i = tempText.find(replaceWord); i != std::string::npos;) {
      tempText.replace(i, replaceWord.size(), replaceBy);
      i = tempText.find(replaceWord, i + replaceBy.size());
    }
  }
  return tempText;
}
}  // namespace TextTemplate

using Model = std::map<std::string, std::map<uint64_t, int>>;  // [TableName][encodedString][integer]

HCBUDOUX_NODISCARD Model loadModel(const std::string &json) {
  // Encode UTF-8 string
  const auto encodeKey = [](const std::string &utf8str) -> uint64_t {
    static const auto utf8strToUtf32vec = [](const std::string &utf8s) -> std::vector<uint32_t> {
      std::vector<uint32_t> utf32s;
      for (size_t i = 0; i < utf8s.size();) {
        uint32_t utf32_char = 0;
        int utf32_char_size_in_bytes = 0;
        {
          const int rest = static_cast<int>(utf8s.size() - i);
          const uint8_t c0 = static_cast<uint8_t>(rest >= 1 ? utf8s[i + 0] : 0);
          const uint8_t c1 = static_cast<uint8_t>(rest >= 2 ? utf8s[i + 1] : 0);
          const uint8_t c2 = static_cast<uint8_t>(rest >= 3 ? utf8s[i + 2] : 0);
          const uint8_t c3 = static_cast<uint8_t>(rest >= 4 ? utf8s[i + 3] : 0);
          if ((c0 & 0x80) == 0) {
            if (rest >= 1) {
              const uint32_t p0 = c0 & 0x7f;
              utf32_char = p0;
              utf32_char_size_in_bytes = 1;
            }
          } else if ((c0 & 0xe0) == 0xc0) {
            if (rest >= 2) {
              const uint32_t p0 = (c0 & 0x1f) << 6;
              const uint32_t p1 = (c1 & 0x3f);
              utf32_char = p0 | p1;
              utf32_char_size_in_bytes = 2;
            }
          } else if ((c0 & 0xf0) == 0xe0) {
            if (rest >= 3) {
              const uint32_t p0 = (c0 & 0x0f) << 12;
              const uint32_t p1 = (c1 & 0x3f) << 6;
              const uint32_t p2 = (c2 & 0x3f);
              utf32_char = p0 | p1 | p2;
              utf32_char_size_in_bytes = 3;
            }
          } else if ((c0 & 0xf8) == 0xf0) {
            if (rest >= 4) {
              const uint32_t p0 = (c0 & 0x07) << 18;
              const uint32_t p1 = (c1 & 0x3f) << 12;
              const uint32_t p2 = (c2 & 0x3f) << 6;
              const uint32_t p3 = (c3 & 0x3f);
              utf32_char = p0 | p1 | p2 | p3;
              utf32_char_size_in_bytes = 4;
            }
          }
        }
        if (utf32_char_size_in_bytes == 0) {
          break;
        }
        if (utf32_char == 0) {
          break;
        }
        utf32s.push_back(utf32_char);
        i += utf32_char_size_in_bytes;
      }
      return utf32s;
    };

    const std::vector<uint32_t> utf32s = utf8strToUtf32vec(utf8str);
    switch (utf32s.size()) {
      case 1:
        return utf32s[0];
      case 2:
        return (static_cast<uint64_t>(utf32s[0]) << 21) | static_cast<uint64_t>(utf32s[1]);
      case 3:
        return (static_cast<uint64_t>(utf32s[0]) << 42) | (static_cast<uint64_t>(utf32s[1]) << 21) |
               static_cast<uint64_t>(utf32s[2]);
      default:
        return 0;
    }
  };

  Model model;

  json_value_s *const root = json_parse(json.data(), json.size());
  if (!root) {
    return model;
  }
  const json_object_s *object = json_value_as_object(root);
  if (!object) {
    return model;
  }

  //  Structure of BudouX model JSON file:
  //  {
  //      "UW1" : { "a": 1, "b": 2 },
  //      "UW2" : { "c": 3, "d": 4 },
  //      "BW1" : { "ab": 1, "cd": 2 },
  //      "BW2" : { "ef": 3, "gh": 4 },
  //      "TW1" : { "abc": 1, "def": 2 },
  //      "TW2" : { "ghi": 3, "jkl": 4 }
  //  }
  //
  //  tableName   : "UW1", "UW2", ... , "TW2"
  //  elemName    : "a", "b", ... , "jkl"
  //  elemValue   : 1, 2, ...

  for (const json_object_element_s *topElem = object->start; topElem; topElem = topElem->next) {
    const json_object_element_s *const table = topElem;
    const std::string tableName(table->name->string, table->name->string + table->name->string_size);
    const json_object_s *const tableObject = json_value_as_object(table->value);
    if (!tableObject) {
      model = {};
      break;
    }
    for (const json_object_element_s *p = tableObject->start; p; p = p->next) {
      const json_value_s *const value = p->value;
      if (!value) {
        continue;
      }
      if (value->type != json_type_number) {
        continue;
      }
      const std::string elemName(p->name->string, p->name->string + p->name->string_size);
      const auto *const jn = static_cast<const json_number_s *>(value->payload);
      const std::string elemValue(jn->number, jn->number + jn->number_size);

      model[std::string{tableName}][encodeKey(elemName)] = std::stoi(std::string{elemValue});
    }
  }

  free(root);
  return model;
}

HCBUDOUX_NODISCARD TextTemplate::Dictionary generateTemplateDictionary() {
  struct Language {
    std::string jsonFilename;
    std::string symbol;
  };

  static const Language languages[] = {
      {"ja.json", "ja"},           {"ja_knbc.json", "ja_knbc"}, {"th.json", "th"},
      {"zh-hans.json", "zh_hans"}, {"zh-hant.json", "zh_hant"},
  };

  const auto generateTemplateName = [](const std::string &name) -> std::string {
    return "HCBUDOUX_IMPL_TEMPLATE(" + name + ")";
  };

  const auto itemCodeToString = [](const std::string &tableName, uint64_t encoded) -> std::string {
    char buf[64];
    switch (tableName[0]) {
      case 'U':
        snprintf(buf, sizeof(buf), "0x%08x", static_cast<uint32_t>(encoded));
        return buf;
      case 'B':  // fallthrough
      case 'T':
        snprintf(buf, sizeof(buf), "UINT64_C(0x%016" PRIx64 ")", encoded);
        return buf;
      default:
        return "";
    }
  };

  const auto itemScoreToString = [](int score) -> std::string {
    char buf[64];
    snprintf(buf, sizeof(buf), "%+6d", score);
    return buf;
  };

  TextTemplate::Dictionary templateMap;

  for (const Language &language : languages) {
    const std::string jsonFilename = "../third_party/budoux/budoux/models/" + language.jsonFilename;
    const std::string json = readFile(jsonFilename);
    if (json.empty()) {
      fprintf(stderr, "Failed to load %s\n", jsonFilename.c_str());
      return {};
    }
    const Model model = loadModel({json.data(), json.size()});
    int baseScore = 0;

    for (const auto &table : model) {
      const auto &tableName = table.first;  // "UW1"
      const auto &elements = table.second;  // ["A"] = 1, ["B"] = 2, ...

      std::string items;
      int count = 0;

      for (const auto &element : elements) {
        const uint64_t elementEncodedName = element.first;
        const int elementScore = element.second;

        if (count++ % 4 == 0) {
          items += "\n        ";
        }

        items += "{";
        items += itemCodeToString(tableName, elementEncodedName);  // UINT64_C(0x...)
        items += ",";
        items += itemScoreToString(elementScore);  // +123
        items += "},";

        baseScore += elementScore;
      }

      const std::string key = generateTemplateName("_" + language.symbol + "_." + tableName);
      templateMap[key] = items;
    }

    {
      const std::string key = generateTemplateName("_" + language.symbol + "_.Base");
      templateMap[key] = itemScoreToString(-baseScore);
    }
  }

  return templateMap;
}

HCBUDOUX_NODISCARD bool generate() {
  const std::string templateFilename = "./hcbudoux.template.h";
  const std::string outFilename = "../include/hcbudoux.h";
  const TextTemplate::Dictionary templateMap = generateTemplateDictionary();
  if (templateMap.empty()) {
    return false;
  }
  const std::string outStr = TextTemplate::replaceAll(readFile(templateFilename), templateMap);
  if (outStr.empty()) {
    return false;
  }
  FILE *fp = fopen(outFilename.c_str(), "wb");
  if (!fp) {
    return false;
  }
  const size_t written = fwrite(outStr.data(), sizeof(outStr[0]), outStr.size(), fp);
  fclose(fp);
  if (written != outStr.size()) {
    return false;
  }
  return true;
}

int main(int, const char *[]) { return generate() ? EXIT_SUCCESS : EXIT_FAILURE; }
