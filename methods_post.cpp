#include "webserv.hpp"
#include <string>
#include <dirent.h>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <unistd.h>
#include <fcntl.h>


int extract_number(const char *name) 
{
	const char *prefix = "note_";
	const char *suffix = ".txt";
	int i = 0;
	int j = 0;
	bool found_digit = false;
	int res = 0;
	while (prefix[i]) //check prefix match
	{
        	if (name[i] != prefix[i])
			return -1;
		i++;
	}
	while(name[i] >= '0' && name[i] <= '9') //parse number
	{
		found_digit = true;
		res = res * 10 + (name[i] - '0');
		i++;
	}
	if (!found_digit) //check if number found
		return -1;
	while (suffix[j] && name[i + j]) //check suffix match
	{
		if (name[i + j] != suffix[j])
			return -1;
		j++;
	}
	if (suffix[j] != '\0' || name[i + j] != '\0') //check that both names end here
		return -1;
	return res;
}

int init_counter_from_dir(const std::string &upload_dir)
{
    DIR *dir = opendir(upload_dir.c_str());
    if (!dir)
        return 0; // directory couldnt be opened OJO it should definitely exist, otherwise parser should throw an error and server shouldnt be started in the first place

    struct dirent *entry;
    int max_i;
    int i;

    max_i = 0;
    i = 0;
    while ((entry = readdir(dir)))
    {
        i = extract_number(entry->d_name); //find the number of the note in its filename
            if (i >= 0 && i > max_i) //in case of better match and no error in extract number, update
                max_i = i;
    }
    closedir(dir);
    return max_i + 1; // set it to the next available slot
}


std::string	gen_filename(t_location *l)
{
	if (!l->upload_count) //in case upload count is set to 0, check if theres any uploads from running server previously
		l->upload_count = init_counter_from_dir(l->upload_store);
	std::stringstream sstr;
	sstr << "note_" << l->upload_count << ".txt";
	std::string filename(sstr.str());
	l->upload_count++;
	return filename;
		
}

std::string handle_post(Client &client, const t_server &config, t_location *l)
{
	printf("POST WAS CALLED\n");
	int fd;
	ssize_t write_count;

	if (!l->upload_enabled) //check if upload in this location is enabled
	{
		client.set_error_code(403);
		return std::string();
	}
	else if (client.get_request().body.size() > config.max_bdy_size) //check body size
	{
		client.set_error_code(413);
		return std::string();
	}
	std::string path = std::string(l->upload_store) + "/" + gen_filename(l); //append name of new file to the path of the upload directory
	fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644); //open file write only, if it doesnt exist, create it, if it exists, erase its contents, number stands for permissions, we can read & write, client can read
	if (fd == -1)
	{
		client.set_error_code(500);
		return std::string();
	}
	const std::string &body = client.get_request().body;
	write_count = write(fd, body.c_str(), body.size());
	if (write_count < 0 || write_count != (ssize_t)body.size())
		client.set_error_code(500);
	else
		printf("post didnt encounter any issues\n");
	close(fd);
	return std::string();
}
