#include "webserv.hpp"
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

//TODO include more headers

#include "webserv.hpp"
#include <string>
#include <dirent.h>
#include <sys/stat.h>
#include <cstdlib>
#include <ctime>
#include <sstream>

static void seed_random()
{
    static int seeded = 0;
    if (!seeded)
    {
        std::srand(std::time(NULL));
        seeded = 1;
    }
}

// simple check for .png (case-insensitive)
static int is_png(const char *name)
{
    int len = 0;
    while (name[len]) len++;
    if (len < 4) return 0;

    char c1 = name[len-4]; char c2 = name[len-3];
    char c3 = name[len-2]; char c4 = name[len-1];

    if (c1 != '.' || (c2 != 'p' && c2 != 'P') || (c3 != 'n' && c3 != 'N') || (c4 != 'g' && c4 != 'G'))
        return 0;
    return 1;
}

// list PNG files in directory
static int list_pngs(const std::string &dirpath, std::string files[], int max_files)
{
    DIR *dp = opendir(dirpath.c_str());
    if (!dp) return 0;

    struct dirent *entry;
    int count = 0;
    struct stat st;
    while ((entry = readdir(dp)) && count < max_files)
    {
        if (entry->d_name[0] == '.') continue; // skip . and ..
        if (!is_png(entry->d_name)) continue;

        std::string full = dirpath + "/" + entry->d_name;
        if (stat(full.c_str(), &st) == 0 && (st.st_mode & S_IFREG))
        {
            files[count] = entry->d_name;
            count++;
        }
    }
    closedir(dp);
    return count;
}

std::string name_pumpkin(Client &client, t_location *l)
{
    (void)client;

    seed_random();

    // pick a random pumpkin
    std::string files[100];
    int num_files = list_pngs(std::string(l->root) + "/pumpkins", files, 100);
    std::string chosen = "";
    if (num_files > 0)
        chosen = files[std::rand() % num_files];

    // path to template file
    std::string template_path = std::string(l->root) + "/name_pumpkin.html";

    // read file into string
    std::string html = file_to_str(client, template_path); // reuse your file_to_str()

    // replace placeholder {{PUMPKIN_IMG}} with actual filename
    std::string placeholder = "{{PUMPKIN_IMG}}";
    size_t pos = html.find(placeholder);
    if (pos != std::string::npos)
        html.replace(pos, placeholder.size(), chosen);

    // optional: if no pumpkin available, replace with message
    if (chosen == "")
    {
        std::string no_pumpkin_msg = "<p>No pumpkins available right now!</p>";
        pos = html.find("{{NO_PUMPKIN_MSG}}");
        if (pos != std::string::npos)
            html.replace(pos, std::string("{{NO_PUMPKIN_MSG}}").size(), no_pumpkin_msg);
    }
    else
    {
        // remove NO_PUMPKIN_MSG placeholder if pumpkin exists
        pos = html.find("{{NO_PUMPKIN_MSG}}");
        if (pos != std::string::npos)
            html.replace(pos, std::string("{{NO_PUMPKIN_MSG}}").size(), "");
    }

    return html;
}

std::string autoindex_directory(Client &client, const std::string path)
{
	(void)client;
	(void)path;
	return std::string();
	//open the directory q opendir()
	//loop through entries w readdir()
	//generate a html line in an html string for each entry
	//close the directory w closedir()
	//return the html str as the body
}

std::string	file_to_str(Client &client, const std::string &path)
{
	std::string content_bytes; //can be chars in case its a text file or compressed image data in case its a picture
	int fd = open(path.c_str(), O_RDONLY);
	if (-1 == fd)
	{
		client.set_error_code(404);
		printf("404 set at third instance\n");
		return content_bytes;
	}
	char buffer[4096]; //4kb
	ssize_t bytes_read; //ssize_t is signed size_t, which is useful for -1 error
	while ((bytes_read = read(fd, buffer, 4096)) > 0) //read bytes from file
		content_bytes.append(buffer, bytes_read); //keep appending them
	if (bytes_read == -1) //handle read error
	{
		client.set_error_code(500);
		content_bytes.clear();
	}
	close(fd);
	return content_bytes; //return buffer
}

std::string handle_get(Client &client, const t_server &config, t_location *l)
{
	(void)config; //TODO remove from ft if not needed
	struct stat st; //struct that stat fills with information about a file path
	printf("root is: '%s'\n", l->root);
	printf("uri is: '%s'\n", client.get_path().c_str());
	if (client.get_path() == "/name_pumpkin.html")
		return name_pumpkin(client, l);
	std::string path = std::string(l->root) + client.get_path();
	printf("index is being searched at this location : '%s'\n", path.c_str());
	if (stat(path.c_str(), &st) == -1) //if stat returns -1, the file doesnt exist, path not found
	{
		client.set_error_code(404);
		printf("404 set at second instance\n");
		return std::string();
	}
	if (S_ISREG(st.st_mode)) //S_ISREG is the flag for a regular file, not a direcory
		return file_to_str(client, path);
	else if (S_ISDIR(st.st_mode)) //handle directories
	{
		if (l->autoindex) //if autoindexing is turned on, list files in directory
			return autoindex_directory(client, path); //TODO write this
		else if (l->default_file) //if its not on and theres a default file, serve it
		{
			std::string index_path = std::string(l->root) + "/" + std::string(l->default_file);
			if (stat(index_path.c_str(), &st) == 0 && S_ISREG(st.st_mode))
				return file_to_str(client, index_path);
		}
	}
	client.set_error_code(403); //autoindex off && no default fine || path is neither file nor dir
	return std::string();
}
