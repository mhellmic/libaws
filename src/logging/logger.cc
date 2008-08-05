#include "logging/logger.hh"

#include <time.h>
#include <cassert>
#include "logging/loggermanager.hh"
#include "logging/loggerconfig.hh"


namespace logging {
  bool Logger::DO_XML_LOG=true;

  Logger::Logger(const std::string& loggerName) : theLoggerName(loggerName){
	theLoggerPtr = LoggerManager::logmanager()->registerLogger(loggerName);
}

int Logger::getLevel() const{
	return theLoggerPtr->getLogLevel();
}

void Logger::logMessage(int level, const std::string& message, uint32_t line) {
	if(getLevel() <= level)
	{
		//get and format the current time
		time_t rawtime;
  		struct tm * timeinfo;
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );
		std::string formattedTime = asctime(timeinfo);
		formattedTime = formattedTime.substr(0,formattedTime.length()-1);
		std::stringstream logMessage;
		logMessage << formattedTime << '\t';
    if(LoggerManager::logmanager()->getLogTheLoggerName())
    {
      long width = LoggerManager::logmanager()->getLongestLoggerNameLength()-theLoggerName.length();
      assert(width>=0); //sanity
      logMessage << theLoggerName << std::string(width, ' ') << '\t';
    }
		logMessage << "LogLevel=" << level << '\t' << message << std::endl ;
		std::string lMessage(logMessage.str().c_str());
	  theLoggerPtr->logMessage(lMessage);

    if(DO_XML_LOG){
      std::stringstream log4jMessage;
      log4jMessage << "<log4j:event logger=\"";
      log4jMessage << replaceAll(theLoggerName, "::", ".");
      log4jMessage << "\" timestamp=\"" << rawtime << "\" sequenceNumber=\"" << ++(LoggerManager::logmanager()->theLogId) << "\" level=\"";
      switch(level){
        case 1:
          log4jMessage << "INFO\"";
          break;
        case 2:
          log4jMessage << "DEBUG\"";
          break;
        case 3:
          log4jMessage << "ERROR\"";
          break;
          }
          log4jMessage << " thread=\"\">\r\n";

       log4jMessage << "<log4j:locationInfo class=\"dummy\"  method=\"dummy\" file=\"dummy\" line=\"" << line << "\"/>\r\n";


       log4jMessage << "<log4j:message><![CDATA[" << message << "]]></log4j:message>\r\n";


       log4jMessage << "</log4j:event>\r\n\r\n" << std::endl;
       std::string lXMLMessage(log4jMessage.str().c_str());
       theLoggerPtr->logXMLMessage(lXMLMessage);

    }
	}
}


std::string Logger::replaceAll( const std::string& s, const std::string& f, const std::string& r ) {
  if ( s.empty() || f.empty() || f == r || s.find(f) == std::string::npos ) {
    return s;
  }
  std::ostringstream build_it;
  size_t i = 0;
  for ( size_t pos; ( pos = s.find( f, i ) ) != std::string::npos; ) {
    build_it.write( &s[i], pos - i );
    build_it << r;
    i = pos + f.size();
  }
  if ( i != s.size() ) {
    build_it.write( &s[i], s.size() - i );
  }
  return build_it.str();
}


void Logger::setLevel(int newLevel)
{
	LoggerManager::logmanager()->setLevel(theLoggerName, newLevel);
}
void Logger::setFileName(const std::string& newFileName)
{
	LoggerManager::logmanager()->setFileName(theLoggerName, newFileName);
}

Logger::~Logger() {}

} //end of namespace logging
