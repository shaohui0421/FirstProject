#include "application.h"

int main(int argc, char* argv[])
{
    Application* app;
    init_log(RCC_LOG_FILEPATH, RCC_LOG_FILE_MAXSIZE, USER_DEBUG, NULL);

    pre_check();

	app = Application::get_application();
    app->main(argc, argv);
    delete app;
    return 0;
}
