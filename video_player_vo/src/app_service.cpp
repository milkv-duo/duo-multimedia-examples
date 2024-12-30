#include "app_service.h"

void AppService::initialize(Application &self) {
    // 初始化应用程序
    loadConfiguration();
    ServerApplication::initialize(self);
    INITLOG();
}

void AppService::uninitialize() {
    // 清理资源
    FLUSHLOG();
    ServerApplication::uninitialize();
}

void AppService::defineOptions(Poco::Util::OptionSet &options) {
    ServerApplication::defineOptions(options);

    options.addOption(
        Poco::Util::Option("help", "h", "Display help information.")
            .required(false)
            .repeatable(false)
            .callback(Poco::Util::OptionCallback<AppService>(
                this, &AppService::handleHelp)));

    options.addOption(
        Poco::Util::Option("infile", "i", "Specify input file path.")
            .required(true)
            .repeatable(false)
            .argument("<infile>")
            .callback(Poco::Util::OptionCallback<AppService>(
                this, &AppService::handleInFile)));
}

void AppService::handleHelp(const std::string &name, const std::string &value) {
    displayHelp();
    stopOptionsProcessing();
    _helpRequested = true;
}

void AppService::handleInFile(const std::string &name,
                              const std::string &value) {
    _input_file = value;
}

void AppService::displayHelp() {
    Poco::Util::HelpFormatter helpFormatter(options());
    helpFormatter.setCommand(commandName());
    helpFormatter.setUsage("[-h] [-i record.flv]");
    helpFormatter.setHeader(
        "This is a simple video player program based on milkV duo.");
    helpFormatter.setFooter(
        "For more information, visit the website https://milkv.io.");
    helpFormatter.format(std::cout);
}

int AppService::main(const std::vector<std::string> &args) {

    if (_helpRequested) {
        return Application::EXIT_OK;
    }

    INFO("video player beggin ...");
    INFO("arg: {0}", _input_file);

    MediaPlayer *_player = new MediaPlayer();
    _player->init_cviSdk();
    VideoDecoder *_decoder = new VideoDecoder(_player);
    _decoder->open_file(_input_file);

    Poco::ThreadPool::defaultPool().start(*_player); // start workers
    Poco::ThreadPool::defaultPool().start(*_decoder);

    waitForTerminationRequest();

    Poco::ThreadPool::defaultPool().joinAll();

    delete _decoder;
    delete _player;

    return Application::EXIT_OK;
}