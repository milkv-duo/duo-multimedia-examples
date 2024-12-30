#pragma once

#include "Poco/Exception.h"
#include "Poco/ThreadPool.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/ServerApplication.h"

#include "video_record.h"

class AppService : public Poco::Util::ServerApplication {
  protected:
    void initialize(Application &self);
    void uninitialize();

    void defineOptions(Poco::Util::OptionSet &options);
    void handleHelp(const std::string &name, const std::string &value);
    void handleFormat(const std::string &name, const std::string &value);
    void handleTime(const std::string &name, const std::string &value);
    void handleOutFile(const std::string &name, const std::string &value);

    void displayHelp();

    int main(const std::vector<std::string> &args) override;

  private:
    bool _helpRequested = false;

    RecordConf _conf;
};