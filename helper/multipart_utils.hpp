#pragma once
#include <string>

// Extracts and saves a file from a multipart/form-data POST body
// body: the full POST body (binary-safe)
// boundary: the boundary string (without leading --)
// out_filename: where to save the file
// Returns true on success, false on failure
bool extract_and_save_multipart_file(const std::string& body, const std::string& boundary, const std::string& out_filename);
