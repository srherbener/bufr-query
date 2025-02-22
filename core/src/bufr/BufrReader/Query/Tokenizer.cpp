// (C) Copyright 2023 NOAA/NWS/NCEP/EMC

#include "bufr/Tokenizer.h"

#include <memory>
#include <vector>
#include <string>

namespace bufr {
    std::vector<std::shared_ptr<Token>> Tokenizer::tokenize(const std::string &query)
    {
        // remove whitespace from query
        const auto cleanedStr = std::regex_replace(query, std::regex("\\s"), "");

        std::vector<std::shared_ptr<Token>> tokens;

        std::string::const_iterator iter = cleanedStr.begin();
        std::string::const_iterator end = cleanedStr.end();

        std::shared_ptr<Token> token = nullptr;
        if ((token = MultiQueryToken::parse(iter, end)))
        {}
        else if ((token = QueryToken::parse(iter, end)))
        {}
        else
        { std::cout << "Tokenizer::tokenize: no match for " << query << std::endl; }

        tokens.push_back(token);

        return tokens;
    }
}  // namespace bufr
