#include "multipart_utils.hpp"

std::string extract_filename_from_disposition(const std::string& header)
{
    std::string key = "filename=\"";
    size_t start = header.find(key);
    if (start == std::string::npos)
	{
		printf("RETURN HERE \n");
        return "";
	}
    start += key.length();
    size_t end = header.find('"', start);
    if (end == std::string::npos) {
        size_t max_search = std::min(header.size(), start + 200);
        for (size_t i = start; i < max_search; ++i) {
            if (header[i] == '"') {
                end = i;
                break;
            }
        }
        if (end == std::string::npos)
            return "";
    }
    std::string filename = header.substr(start, end - start);
    return filename;
}

std::string extract_boundary_from_disposition(const std::string& content_type)
{
	std::string boundary_key = "boundary=";
	
    size_t bpos = content_type.find(boundary_key);
    if (bpos == std::string::npos)
		return "";
	
	std::string  boundary = content_type.substr(bpos + boundary_key.length());
	size_t end = boundary.find_first_of(" ;\r\n");
    if (end != std::string::npos)
		boundary =  boundary.substr(0, end);
	return boundary;
}

bool extract_and_save_multipart_file(const std::string& body, const std::string& boundary, const std::string& out_filename)
{
    std::string disp = "Content-Disposition: form-data;";
    size_t disp_pos = body.find(disp);
    if (disp_pos == std::string::npos)
		return false;

    size_t fn_pos = body.find("filename=\"", disp_pos);
    if (fn_pos == std::string::npos)
		return false;

    fn_pos += 10; // skip 'filename="'
    size_t fn_end = body.find('"', fn_pos);
    if (fn_end == std::string::npos)
		return false;

    // Find start of file data
    size_t content_type = body.find("Content-Type:", fn_end);
    if (content_type == std::string::npos)
		return false;
    size_t data_start = body.find("\r\n\r\n", content_type);
    if (data_start == std::string::npos)
		data_start = body.find("\n\n", content_type);
    if (data_start == std::string::npos)
		return false;
    data_start += (body[data_start] == '\r') ? 4 : 2; // skip "\r\n\r\n" or "\n\n"

    // Find end of file data (next boundary)
    std::string boundary_marker1 = "\r\n--" + boundary;
    std::string boundary_marker2 = "--" + boundary;
    size_t data_end = body.find(boundary_marker1, data_start);
    if (data_end == std::string::npos) {
        data_end = body.find(boundary_marker2, data_start);
    }
    if (data_end == std::string::npos)
		return false;
    std::ofstream out(out_filename.c_str(), std::ios::binary);
    if (!out) return false;
    out.write(&body[data_start], data_end - data_start);
    out.close();
//    fprintf(stderr, "[multipart_utils] Wrote %zu bytes to '%s'\n", data_end - data_start, out_filename.c_str());
    return true;
}


std::string make_unique_filename(const std::string& filename) {
    std::ostringstream oss;
    oss << std::time(NULL) << "_" << filename;
    return oss.str();
}