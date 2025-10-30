#include "multipart_utils.hpp"
#include <fstream>
#include <string>
#include <cstdio>

bool extract_and_save_multipart_file(const std::string& body, const std::string& boundary, const std::string& out_filename) {
    fprintf(stderr, "[multipart_utils] body size=%zu, boundary='%s'\n", body.size(), boundary.c_str());
    std::string file_header = "Content-Disposition: form-data; name=\"photo\"; filename=\"";
    size_t file_part = body.find(file_header);
    if (file_part == std::string::npos) {
        size_t show = std::min<size_t>(body.size(), 200);
        return false;
    }

    fprintf(stderr, "[multipart_utils] file header at pos=%zu\n", file_part);
    // Find start of file data
    size_t content_type = body.find("Content-Type:", file_part);
    if (content_type == std::string::npos) {
        return false;
    }
    size_t data_start = body.find("\r\n\r\n", content_type);
    if (data_start == std::string::npos) {
        return false;
    }
    data_start += 4; // skip "\r\n\r\n"

    // Find end of file data (next boundary)
    std::string boundary_marker1 = "\r\n--" + boundary;
    std::string boundary_marker2 = "--" + boundary; // sometimes at start it appears without preceding CRLF
    size_t data_end = body.find(boundary_marker1, data_start);
    if (data_end == std::string::npos) {
        data_end = body.find(boundary_marker2, data_start);
        if (data_end != std::string::npos) {
            fprintf(stderr, "[multipart_utils] found boundary without CRLF at pos=%zu\n", data_end);
        }
    } else {
        fprintf(stderr, "[multipart_utils] found boundary with CRLF at pos=%zu\n", data_end);
    }
    if (data_end == std::string::npos) {
        fprintf(stderr, "[multipart_utils] boundary not found after data_start\n");
        size_t show = std::min<size_t>(body.size() - data_start, 200);
        fprintf(stderr, "[multipart_utils] tail snippet (from data_start):\n%.*s\n", (int)show, body.c_str() + data_start);
        return false;
    }

    // Write file data
    std::ofstream out(out_filename.c_str(), std::ios::binary);
    if (!out) {
        fprintf(stderr, "[multipart_utils] Failed to open output file '%s'\n", out_filename.c_str());
        return false;
    }
    out.write(&body[data_start], data_end - data_start);
    out.close();
    fprintf(stderr, "[multipart_utils] Wrote %zu bytes to '%s'\n", data_end - data_start, out_filename.c_str());
    return true;
}
