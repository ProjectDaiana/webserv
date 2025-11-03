#ifndef MULTIPARTS_UTILS_HPP
#define MULTIPARTS_UTILS_HPP

#include <string>
#include <fstream>
#include <string>
#include <cstdio>
#include <string>
#include <sstream>
#include <ctime>

bool extract_and_save_multipart_file(const std::string& body, const std::string& boundary, const std::string& out_filename);
std::string extract_filename_from_disposition(const std::string& header);
std::string extract_boundary_from_disposition(const std::string& content_type);
std::string make_unique_filename(const std::string& filename);

#endif