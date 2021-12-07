#include "mainwindow.h"
#include "appconfig.h"
#include "applog.h"

bool MainWindow::getCurrentTemper()
{
    FILE* thermal;
    int n = 0;
#ifdef Q_OS_LINUX
    const char* temper_file_name = "/sys/class/thermal/thermal_zone0/temp";
#else
    const char* temper_file_name = "d:/temp.txt";
#endif

    thermal = fopen(temper_file_name, "r");

    if (!thermal)
        return false;

    int rv = 0;
    n = fscanf(thermal, "%d", &rv);
    fclose(thermal);
    appState.temper = static_cast<quint16>(rv);
    return true;
}

void MainWindow::readCPUtemper()
{

    int onTemper  = appConfig::value("FAN/StartTemper").toInt() ;
    int offTemper = appConfig::value("FAN/StopTemper" ).toInt() ;
    onTemper  = 1000 * qMax(onTemper, 45);
    offTemper = 1000 * qMax(offTemper, 40);

    if (!getCurrentTemper())
    {
        appState.temper = static_cast<quint16>(onTemper + 1);
        if (appState.fanState != FAN_ON)
        {
            appLog::write(LOG_LEVEL_FAN_CTRL, "error get temperature from mon file. switch fan ON");
            fanSwitch(true);
            return;
        }
    }

    if ((appState.temper >= onTemper) )
    {
        fanSwitch(true);
        return;
    }

    if ((appState.temper <= offTemper) && (appState.fanState != FAN_OFF))
    {
        fanSwitch(false);
        return;
    }

}

bool MainWindow::fanSwitch(bool on)
{
    quint8 newState = on ? FAN_ON : FAN_OFF;
    if (appState.fanState != newState)
    {
        QString str = tr("Temper  %1 fan is %2").arg(appState.temper).arg(on ? "started" : "stopped");
        appLog::write(LOG_LEVEL_FAN_CTRL, str);
        appState.fanState = newState;
        digitalWrite(PIN_FAN, 1);
        return true;
    }
    return false;
}


