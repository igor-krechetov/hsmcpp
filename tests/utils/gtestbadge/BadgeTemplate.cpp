/**
 * This file was generated by bin2cpp v2.1.0
 * Copyright (C) 2013-2018 end2endzone.com. All rights reserved.
 * bin2cpp is open source software, see http://github.com/end2endzone/bin2cpp
 * Source code for file 'BadgeTemplate.svg', last modified 1534620256.
 * Do not modify this file.
 */
#include "BadgeTemplate.h"

#include <stdio.h>  //for FILE

#include <string>  //for memcpy
namespace bin2cpp {
class BadgeTemplateFile : public virtual bin2cpp::File {
public:
    BadgeTemplateFile() {}
    ~BadgeTemplateFile() {}
    virtual size_t getSize() const {
        return 1286;
    }
    virtual const char* getFilename() const {
        return "BadgeTemplate.svg";
    }
    virtual const char* getBuffer() const {
        const char* buffer =
            "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" width=\"{width}\" "
            "height=\"{height}\">\r\n  <linearGradient id=\"b\" x2=\"0\" y2=\"100%\">\r\n    <stop offset=\"0\" "
            "stop-color=\"#bbb"
            "\" stop-opacity=\".1\"/>\r\n    <stop offset=\"1\" stop-opacity=\".1\"/>\r\n  </linearGradient>\r\n  <clipPath "
            "id=\"a\">\r\n    <rect width=\"{width}\" height=\"{height}\" rx=\"3\" fill=\"#fff\"/>\r\n  </clipPath>\r\n  <g "
            "clip-pa"
            "th=\"url(#a)\">\r\n    <path fill=\"{left.back_color}\" d=\"M0 0h{left.width}v{height}H0z\"/>\r\n    <path "
            "fill=\"{right.back_color}\" d=\"M{left.width} 0h{right.width}v{height}H{left.width}z\"/>\r\n    <path fill=\"u"
            "rl(#b)\" d=\"M0 0h{width}v{height}H0z\"/>\r\n  </g>\r\n  <g fill=\"{left.front_color}\" text-anchor=\"middle\" "
            "font-family=\"Verdana,Geneva,sans-serif\" font-size=\"{left.font_size}\">\r\n{icon}    <text x=\"{left.text"
            ".x}\" y=\"{left.shadow.height}\" fill=\"#010101\" fill-opacity=\".3\">{left.text}</text>\r\n    <text "
            "x=\"{left.text.x}\" y=\"{left.text.height}\">{left.text}</text>\r\n  </g>\r\n  <g fill=\"{right.front_color}\" "
            "text-a"
            "nchor=\"middle\" font-family=\"Verdana,Geneva,sans-serif\" font-size=\"{right.font_size}\">\r\n    <text "
            "x=\"{right.text.x}\" y=\"{right.shadow.height}\" fill=\"#010101\" fill-opacity=\".3\">{right.text}</text>\r\n    "
            "<text x=\"{right.text.x}\" y=\"{right.text.height}\">{right.text}</text>\r\n  </g>\r\n</svg>\r\n";
        return buffer;
    }
    virtual bool save(const char* iFilename) const {
        FILE* f = fopen(iFilename, "wb");
        if (!f)
            return false;
        size_t fileSize = getSize();
        const char* buffer = getBuffer();
        fwrite(buffer, 1, fileSize, f);
        fclose(f);
        return true;
    }
};
const File& getBadgeTemplateFile() {
    static BadgeTemplateFile _instance;
    return _instance;
}
};  // namespace bin2cpp
