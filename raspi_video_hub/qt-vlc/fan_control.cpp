#include "mainwindow.h"
#include "appconfig.h"
#include "applog.h"

void MainWindow::readCPUtemper()
{

    FILE* thermal;
    int n = 0;
#ifdef Q_OS_LINUX
    const char* temper_file_name = "/sys/class/thermal/thermal_zone0/temp";
#else
    const char* temper_file_name = "d:/temp.txt";
#endif

    int onTemper  = appConfig::value("FAN/StartTemper").toInt() ;
    int offTemper = appConfig::value("FAN/StopTemper" ).toInt() ;
    onTemper  = 1000 * qMax(onTemper, 45);
    offTemper = 1000 * qMax(offTemper, 40);

    thermal = fopen(temper_file_name, "r");

    if (thermal)
    {
        int rv = 0;
        n = fscanf(thermal, "%d", &rv);
        fclose(thermal);
        appState.temper = static_cast<quint16>(rv);

    }

    if ( !thermal || !n )
    {
        appState.temper = static_cast<quint16>(onTemper + 1);
        if (!appState.fanState) // Вентилятор выключен пишем в лог и включаем принудительно
            appLog::write(0, "error get temperature from mon file. switch fan ON");
    }

    //qDebug("current temp %u  fan state %d onTemp %d offTemp %d ",(unsigned int)appState.temper, (int)appState.fanState,onTemper,offTemper);
    bool fan_state_changed = false;
    if ((appState.temper >= onTemper) && (appState.fanState != 1))
    {
        appState.fanState = 1;
        fan_state_changed = true;
        QString str = tr("Temper  %1 > %2, starting fan").arg(appState.temper).arg(onTemper);
        appLog::write(6, str);
    }

    if ((appState.temper <= offTemper) && (appState.fanState != 0))
    {
        appState.fanState = 0;
        fan_state_changed = true;
        QString str = tr("Temper  %1 < %2, stopping fan").arg(appState.temper).arg(onTemper);
        appLog::write(6, str);
    }

    if (fan_state_changed)
    {
        digitalWrite(PIN_FAN, appState.fanState);
    }
}

