#pragma once

#include "Poco/Exception.h"
#include "Poco/ThreadPool.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/SignalHandler.h"

#include "media_player.h"

class AppService : public Poco::Util::ServerApplication {
  protected:
    void initialize(Application &self);
    void uninitialize();

    void defineOptions(Poco::Util::OptionSet &options);
    void handleHelp(const std::string &name, const std::string &value);
    void handleInFile(const std::string &name, const std::string &value);

    void displayHelp();

    int main(const std::vector<std::string> &args) override;

  private:
    bool _helpRequested = false;

    std::string _input_file;

};