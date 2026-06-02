#ifndef STEGANO_H
#define STEGANO_H

#include <string>
bool embed_lsb_bmp(const std::string& input_bmp, const std::string& output_bmp, const std::string& message);
std::string extract_lsb_bmp(const std::string& bmp_file);

#endif
