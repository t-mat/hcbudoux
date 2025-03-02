#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS 1
#endif
#include <json.h>  // https://github.com/sheredom/json.h/blob/master/json.h

#include <map>
#include <string>
#include <vector>

std::string readFile(const std::string &filename) {
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

std::string replaceAll(const std::string &src, const Dictionary &dictionary) {
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

Model loadModel(const std::string &json) {
  // Encode UTF-8 string
  const auto encodeKey = [](const std::string &utf8str) -> uint64_t {
    static const auto utf8strToUtf32vec = [](const std::string &utf8s) -> std::vector<uint32_t> {
      std::vector<uint32_t> utf32s;
      for (size_t i = 0; i < utf8s.size();) {
        uint32_t utf32_char = 0;
        int utf32_char_size_in_bytes = 0;
        {
          int const rest = static_cast<int>(utf8s.size() - i);
          uint8_t const c0 = static_cast<uint8_t>(rest >= 1 ? utf8s[i + 0] : 0);
          uint8_t const c1 = static_cast<uint8_t>(rest >= 2 ? utf8s[i + 1] : 0);
          uint8_t const c2 = static_cast<uint8_t>(rest >= 3 ? utf8s[i + 2] : 0);
          uint8_t const c3 = static_cast<uint8_t>(rest >= 4 ? utf8s[i + 3] : 0);
          if ((c0 & 0x80) == 0) {
            if (rest >= 1) {
              uint32_t const p0 = c0 & 0x7f;
              utf32_char = p0;
              utf32_char_size_in_bytes = 1;
            }
          } else if ((c0 & 0xe0) == 0xc0) {
            if (rest >= 2) {
              uint32_t const p0 = (c0 & 0x1f) << 6;
              uint32_t const p1 = (c1 & 0x3f);
              utf32_char = p0 | p1;
              utf32_char_size_in_bytes = 2;
            }
          } else if ((c0 & 0xf0) == 0xe0) {
            if (rest >= 3) {
              uint32_t const p0 = (c0 & 0x0f) << 12;
              uint32_t const p1 = (c1 & 0x3f) << 6;
              uint32_t const p2 = (c2 & 0x3f);
              utf32_char = p0 | p1 | p2;
              utf32_char_size_in_bytes = 3;
            }
          } else if ((c0 & 0xf8) == 0xf0) {
            if (rest >= 4) {
              uint32_t const p0 = (c0 & 0x07) << 18;
              uint32_t const p1 = (c1 & 0x3f) << 12;
              uint32_t const p2 = (c2 & 0x3f) << 6;
              uint32_t const p3 = (c3 & 0x3f);
              utf32_char = p0 | p1 | p2 | p3;
              utf32_char_size_in_bytes = 4;
            }
          }
        }
        if (utf32_char == 0) {
          break;
        }
        utf32s.push_back(utf32_char);
        i += utf32_char_size_in_bytes;
      }
      return utf32s;
    };

    std::vector<uint32_t> const utf32s = utf8strToUtf32vec(utf8str);
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
  const json_object_s *object = json_value_as_object(root);

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
    std::string const tableName(table->name->string, table->name->string + table->name->string_size);
    const json_object_s *const tableObject = json_value_as_object(table->value);
    for (const json_object_element_s *p = tableObject->start; p; p = p->next) {
      const json_value_s *const value = p->value;
      if (!value) {
        continue;
      }
      if (value && value->type != json_type_number) {
        continue;
      }
      std::string const elemName(p->name->string, p->name->string + p->name->string_size);
      const auto *const jn = static_cast<const json_number_s *>(value->payload);
      std::string const elemValue(jn->number, jn->number + jn->number_size);

      model[std::string{tableName}][encodeKey(elemName)] = std::stoi(std::string{elemValue});
    }
  }

  free(root);
  return model;
}

TextTemplate::Dictionary generateTemplateDictionary() {
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
        sprintf(buf, "0x%08x", static_cast<uint32_t>(encoded));
        return buf;
      case 'B':  // fallthrough
      case 'T':
        sprintf(buf, "UINT64_C(0x%016" PRIx64 ")", encoded);
        return buf;
      default:
        return "";
    }
  };

  const auto itemScoreToString = [](int score) -> std::string {
    char buf[64];
    sprintf(buf, "%+6d", score);
    return buf;
  };

  TextTemplate::Dictionary templateMap;

  for (const Language &language : languages) {
    std::string const jsonFilename = "../third_party/budoux/budoux/models/" + language.jsonFilename;
    std::string const json = readFile(jsonFilename);
    Model const model = loadModel({json.data(), strlen(json.data())});
    int baseScore = 0;

    for (auto const &table : model) {
      auto const &tableName = table.first;  // "UW1"
      auto const &elements = table.second;  // ["A"] = 1, ["B"] = 2, ...

      std::string items;
      int count = 0;

      for (auto const &element : elements) {
        uint64_t const elementEncodedName = element.first;
        int const elementScore = element.second;

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

      std::string const key = generateTemplateName("_" + language.symbol + "_." + tableName);
      templateMap[key] = items;
    }

    {
      std::string const key = generateTemplateName("_" + language.symbol + "_.Base");
      templateMap[key] = itemScoreToString(-baseScore);
    }
  }

  return templateMap;
}

bool generate() {
  std::string const templateFilename = "./hcbudoux.template.h";
  std::string const outFilename = "../include/hcbudoux.h";
  std::string const outStr = TextTemplate::replaceAll(readFile(templateFilename), generateTemplateDictionary());
  FILE *fp = fopen(outFilename.c_str(), "wb");
  if (!fp) {
    return false;
  }
  fwrite(outStr.data(), sizeof(outStr[0]), outStr.size(), fp);
  fclose(fp);
  return true;
}

int main(int, const char *[]) { return generate() ? EXIT_SUCCESS : EXIT_FAILURE; }
