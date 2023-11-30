#include "Logger.h"

// Logger constructor
Logger::Logger(std::string output_file)
{
	this->output_filename = output_file;
	init_log();
}

void Logger::init_log()
{

	const char* primary_dir_name = this->dir_name.c_str();


	int status = 0;

	if (mkdir(primary_dir_name, 0777) != 0 && errno != EEXIST)
	{
		status = -1;
	}

	// Create the empty log file.
	std::ostringstream filename_stream;
	filename_stream << primary_dir_name << "/" << this->output_filename << ".log";
	this->output_filename = filename_stream.str();
	std::ofstream file_stream;
	file_stream.open(filename_stream.str(), std::ios::out);
	file_stream.close();
	
}

void Logger::log_err(std::string output_string)
{
	errs() << output_string << "\n";
	return;
}

void Logger::log_file(std::string output_string)
{
	std::ofstream output_file;
	output_file.open(this->output_filename.c_str(), std::ios_base::app);
	output_file << output_string << "\n";
	output_file.close();

	return;
}
