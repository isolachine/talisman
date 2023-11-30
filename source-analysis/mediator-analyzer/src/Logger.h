#ifndef LOGGER_H
#define LOGGER_H

// LLVM includes
#include "llvm/Support/raw_ostream.h"

// C++ Includes
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/errno.h>

using namespace llvm;


class Logger	{

	private:
		std::string dir_name = "Logs";
		std::string output_filename;

		void init_log();

	public:

		Logger(std::string filename);

		void log_err(std::string output_string);
		void log_file(std::string output_string);

};

#endif
