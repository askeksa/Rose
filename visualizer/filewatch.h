
#include <string>

#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

class FileWatch {
	std::string filename;
	struct stat filestat;
public:
	FileWatch(const char *name): filename(name) {
		stat(filename.c_str(), &filestat);
	}

	bool changed() {
		struct stat newfilestat;
		stat(filename.c_str(), &newfilestat);
		if (newfilestat.st_mtime != filestat.st_mtime) {
			filestat = newfilestat;
			return true;
		} else {
			return false;
		}
	}

	const char *name() {
		return filename.c_str();
	}

	const char *time_text() {
		return ctime(&filestat.st_mtime);
	}
};
