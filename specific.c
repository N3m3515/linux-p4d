//***************************************************************************
// Automation Control
// File specific.c
// This code is distributed under the terms and conditions of the
// GNU GENERAL PUBLIC LICENSE. See the file LICENSE for details.
// Date 16.04.2021 - Jörg Wendel
//***************************************************************************

#include <dirent.h>
#include <inttypes.h>

#ifndef _NO_RASPBERRY_PI_
#  include <wiringPi.h>
#else
#  include "gpio.h"
#endif

#include "lib/json.h"
#include "specific.h"

volatile int showerSwitch {0};

//***************************************************************************
// Configuration Items
//***************************************************************************

std::list<Daemon::ConfigItemDef> P4d::configuration
{
   // p4d

   { "longitude",                 ctNum,     "8.79",         false, "Daemon", "Längengrad", "" },
   { "latitude",                  ctNum,     "50.3",         false, "Daemon", "Breitengrad", "" },

   { "interval",                  ctInteger, "60",   false, "Daemon", "Intervall der Aufzeichung", "Datenbank Aufzeichung [s]" },
   { "webPort",                   ctInteger, "1111", false, "Daemon", "Port des Web Interfaces", "" },
   { "stateCheckInterval",        ctInteger, "10",   false, "Daemon", "Intervall der Status Prüfung", "Intervall der Status Prüfung [s]" },
   { "arduinoInterval",           ctInteger, "10",   false, "Daemon", "Intervall der Arduino Messungen", "[s]" },
   { "ttyDevice",                 ctString,  "/dev/ttyUSB0", false, "Daemon", "TTY Device zur S-3200", "Beispiel: '/dev/ttyUsb0'" },
   { "eloquence",                 ctBitSelect, "1",          false, "Daemon", "Log Eloquence", "" },

   { "tsync",                     ctBool,    "0",    false, "Daemon", "Zeitsynchronisation", "täglich 3:00" },
   { "maxTimeLeak",               ctInteger, "5",    false, "Daemon", " bei Abweichung über [s]", "Mindestabweichung für Synchronisation in Sekunden" },

   { "aggregateHistory",          ctInteger, "1",    false, "Daemon", "Historie [Tage]", "history for aggregation in days (default 0 days -&gt; aggegation turned OFF)" },
   { "aggregateInterval",         ctInteger, "15",   false, "Daemon", " danach aggregieren über", "aggregation interval in minutes - 'one sample per interval will be build'" },
   { "peakResetAt",               ctString,  "",     true,  "Daemon", "", "" },

   { "consumptionPerHour",        ctNum,     "4",    false, "Daemon", "Pellet Verbrauch / Stoker Stunde", "" },

   { "openWeatherApiKey",         ctString,  "",             false, "Daemon", "Openweathermap API Key", "" },

   // web

   { "webUrl",                    ctString,  "",             false, "WEB Interface", "URL der Visualisierung", "kann mit %weburl% in die Mails eingefügt werden" },
   { "webSSL",                    ctBool,    "",             false, "WEB Interface", "Use SSL for WebInterface" },
   { "haUrl",                     ctString,  "",             false, "WEB Interface", "URL der Hausautomatisierung", "Zur Anzeige des Menüs als Link" },

   { "heatingType",               ctChoice,  "",             false, "WEB Interface", "Typ der Heizung", "" },
   { "style",                     ctChoice,  "dark",         false, "WEB Interface", "Farbschema", "" },
   { "iconSet",                   ctChoice,  "light",        false, "WEB Interface", "Status Icon Set", "" },
   { "background",                ctChoice,  "",             false, "WEB Interface", "Background image", "" },
   { "schema",                    ctChoice,  "schema.jpg",   false, "WEB Interface", "Schematische Darstellung", "" },

   { "chartRange",                ctNum,     "1.5",          true, "WEB Interface", "Chart Range", "" },
   { "chartSensors",              ctNum,     "VA:0x0",       true, "WEB Interface", "Chart Sensors", "" },

   // homectrld MQTT interface

   { "mqttUrl",                   ctString,  "tcp://localhost:1883", false, "Home-Control MQTT Interface", "MQTT Broker Url", "MQTT Instanz für den p4d. Beispiel: 'tcp://127.0.0.1:1883'" },
   { "mqttSensorTopics",          ctString,  TARGET "2mqtt/w1/#, " TARGET "2mqtt/arduino/out",  false, "Home-Control MQTT Interface", "Sensor Topics", "" },

   // node-red MQTT interface

   { "mqttNodeRedUrl",            ctString,  "tcp://localhost:1883", false, "Node-Red Interface", "MQTT Node-Red Broker Url", "Optional. Beispiel: 'tcp://127.0.0.1:1883'" },

   // Home Automation MQTT interface

   { "mqttHassUrl",               ctString,  "",  false, "Home Automation Interface (like Home-Assistant, ...)", "Broker Url", "Optional. Beispiel: 'tcp://127.0.0.1:1883'" },
   { "mqttHassUser",              ctString,  "",  false, "Home Automation Interface (like Home-Assistant, ...)", "User", "" },
   { "mqttHassPassword",          ctString,  "",  false, "Home Automation Interface (like Home-Assistant, ...)", "Password", "" },
   { "mqttDataTopic",             ctString,  "",  false, "Home Automation Interface (like Home-Assistant, ...)", "Data Topic Name", "&lt;NAME&gt; wird gegen den Messwertnamen und &lt;GROUP&gt; gegen den Namen der Gruppe ersetzt. Beispiel: p4d2mqtt/sensor/&lt;NAME&gt;/state" },
   { "mqttSendWithKeyPrefix",     ctString,  "",  false, "Home Automation Interface (like Home-Assistant, ...)", "Adresse übertragen", "Wenn hier ein Präfix konfiguriert ist wird die Adresse der Sensoren nebst Präfix übertragen" },
   { "mqttHaveConfigTopic",       ctBool,    "1", false, "Home Automation Interface (like Home-Assistant, ...)", "Config Topic", "Speziell für HomeAssistant" },

   // mail

   { "mail",                      ctBool,    "0",                    false, "Mail", "Mail Benachrichtigung", "Mail Benachrichtigungen aktivieren/deaktivieren" },
   { "mailScript",                ctString,  "/usr/bin/p4d-mail.sh", false, "Mail", "p4d sendet Mails über das Skript", "" },
   { "stateMailTo",               ctString,  "",                     false, "Mail", "Status Mail Empfänger", "Komma getrennte Empfängerliste" },
   { "stateMailStates",           ctMultiSelect, "",                 false, "Mail", "  für folgende Status", "" },
   { "errorMailTo",               ctString,  "",                     false, "Mail", "Fehler Mail Empfänger", "Komma getrennte Empfängerliste" },

   { "deconzHttpUrl",             ctString,  "",                     false, "DECONZ", "deCONZ HTTP URL", "" },
   { "deconzApiKey",              ctString,  "",                     false, "DECONZ", "deCONZ API key", "" },

   { "homeMaticInterface",        ctBool,    "false",                false, "HomeMatic CCU", "HomeMatic Interface", "" },
};

//***************************************************************************
// P4d Daemon
//***************************************************************************

P4d::P4d()
   : Daemon()
{
   webPort = 1111;

   sem = new Sem(0x3da00001);
   serial = new Serial;
   request = new P4Request(serial);
}

P4d::~P4d()
{
   free(stateMailAtStates);
   delete serial;
   delete request;
   delete sem;
}

//***************************************************************************
// Init/Exit
//***************************************************************************

int P4d::init()
{
   int status = Daemon::init();

   getConfigItem("knownStates", knownStates, "");

   if (!isEmpty(knownStates))
   {
      std::vector<std::string> sStates = split(knownStates, ':');
      for (const auto& s : sStates)
         stateDurations[atoi(s.c_str())] = 0;

      tell(eloAlways, "Loaded (%zu) states [%s]", stateDurations.size(), knownStates);
   }

   sem->p();
   serial->open(ttyDevice);
   sem->v();

   return status;
}

int P4d::exit()
{
   serial->close();

   return Daemon::exit();
}

//***************************************************************************
// Init/Exit Database
//***************************************************************************

cDbFieldDef minValueDef("MIN_VALUE", "minvalue", cDBS::ffInt, 0, cDBS::ftData);
cDbFieldDef rangeEndDef("time", "time", cDBS::ffDateTime, 0, cDBS::ftData);
cDbFieldDef endTimeDef("END_TIME", "endtime", cDBS::ffDateTime, 0, cDBS::ftData);

int P4d::initDb()
{
   int status = Daemon::initDb();

   tableErrors = new cDbTable(connection, "errors");
   if (tableErrors->open() != success) return fail;

   tableMenu = new cDbTable(connection, "menu");
   if (tableMenu->open() != success) return fail;

   tableSensorAlert = new cDbTable(connection, "sensoralert");
   if (tableSensorAlert->open() != success) return fail;

   tableTimeRanges = new cDbTable(connection, "timeranges");  // #TODO - still needed
   if (tableTimeRanges->open() != success) return fail;

   tablePellets = new cDbTable(connection, "pellets");
   if (tablePellets->open() != success) return fail;

   // ----------------

   selectAllMenuItems = new cDbStatement(tableMenu);

   selectAllMenuItems->build("select ");
   selectAllMenuItems->bindAllOut();
   selectAllMenuItems->build(" from %s", tableMenu->TableName());

   status += selectAllMenuItems->prepare();

   // ----------------

   selectMenuItemsByParent = new cDbStatement(tableMenu);

   selectMenuItemsByParent->build("select ");
   selectMenuItemsByParent->bindAllOut();
   selectMenuItemsByParent->build(" from %s where ", tableMenu->TableName());
   selectMenuItemsByParent->bind("PARENT", cDBS::bndIn | cDBS::bndSet);

   status += selectMenuItemsByParent->prepare();

   // ----------------

   selectMenuItemsByChild = new cDbStatement(tableMenu);

   selectMenuItemsByChild->build("select ");
   selectMenuItemsByChild->bindAllOut();
   selectMenuItemsByChild->build(" from %s where ", tableMenu->TableName());
   selectMenuItemsByChild->bind("CHILD", cDBS::bndIn | cDBS::bndSet);

   status += selectMenuItemsByChild->prepare();

   // ------------------

   selectSensorAlerts = new cDbStatement(tableSensorAlert);

   selectSensorAlerts->build("select ");
   selectSensorAlerts->bindAllOut();
   selectSensorAlerts->build(" from %s where state = 'A'", tableSensorAlert->TableName());
   selectSensorAlerts->bind("KIND", cDBS::bndIn | cDBS::bndSet, " and ");

   status += selectSensorAlerts->prepare();

   // ------------------

   selectAllSensorAlerts = new cDbStatement(tableSensorAlert);

   selectAllSensorAlerts->build("select ");
   selectAllSensorAlerts->bindAllOut();
   selectAllSensorAlerts->build(" from %s", tableSensorAlert->TableName());

   status += selectAllSensorAlerts->prepare();

   // ------------------
   // select min(value), time from samples
   //    where address = ? type = ?
   //     and time > ?

   minValue.setField(&minValueDef);
   selectStokerHours = new cDbStatement(tableSamples);

   selectStokerHours->build("select ");
   selectStokerHours->bindTextFree("min(value)", &minValue, "", cDBS::bndOut);
   selectStokerHours->bind("TIME", cDBS::bndOut, ", ");
   selectStokerHours->build(" from %s where ", tableSamples->TableName());
   selectStokerHours->bind("ADDRESS", cDBS::bndIn | cDBS::bndSet);
   selectStokerHours->bind("TYPE", cDBS::bndIn | cDBS::bndSet, " and ");
   selectStokerHours->bindCmp(0, "TIME", 0, ">", " and ");

   status += selectStokerHours->prepare();

   // ------------------
   // state duration
   // select value, text, min(time)
   //  from samples
   //  where
   //    address = 1
   //    and type = 'UD'
   //    and text is not null
   //    and date(time) = curdate()
   //    and time > ?
   //    and vaue != ?

   endTime.setField(&endTimeDef);
   selectStateDuration = new cDbStatement(tableSamples);

   selectStateDuration->build("select ");
   selectStateDuration->bind("VALUE", cDBS::bndOut);
   selectStateDuration->bind("TEXT", cDBS::bndOut, ", ");
   selectStateDuration->bindTextFree("min(time)", &endTime, ", ", cDBS::bndOut);
   selectStateDuration->build(" from %s",  tableSamples->TableName());
   selectStateDuration->build(" where %s = 1", tableSamples->getField("ADDRESS")->getDbName());
   selectStateDuration->build(" and %s = 'UD'", tableSamples->getField("TYPE")->getDbName());
   selectStateDuration->build(" and %s is not null", tableSamples->getField("TEXT")->getDbName());
   selectStateDuration->build(" and date(%s) = curdate()", tableSamples->getField("TIME")->getDbName());
   selectStateDuration->bindCmp(0, "TIME", 0, ">", " and ");
   selectStateDuration->bindCmp(0, "VALUE", 0, "!=", " and ");

   status += selectStateDuration->prepare();

   // ------------------
   // all errors

   selectAllErrors = new cDbStatement(tableErrors);

   selectAllErrors->build("select ");
   selectAllErrors->bindAllOut();
   selectAllErrors->build(" from %s",  tableErrors->TableName());
   selectAllErrors->build(" order by time1 desc");

   status += selectAllErrors->prepare();

   // ------------------
   // pending errors

   selectPendingErrors = new cDbStatement(tableErrors);

   selectPendingErrors->build("select ");
   selectPendingErrors->bindAllOut();
   selectPendingErrors->build(" from %s where %s <> 'quittiert' and (%s <= 0 or %s is null)",
                              tableErrors->TableName(),
                              tableErrors->getField("STATE")->getDbName(),
                              tableErrors->getField("MAILCNT")->getDbName(),
                              tableErrors->getField("MAILCNT")->getDbName());

   status += selectPendingErrors->prepare();

   // ------------------
   // select all pellets

   selectAllPellets = new cDbStatement(tablePellets);
   selectAllPellets->build("select ");
   selectAllPellets->bindAllOut();
   selectAllPellets->build(" from %s", tablePellets->TableName());
   selectAllPellets->build(" order by time");
   status += selectAllPellets->prepare();

   // ------------------
   // select * from samples      (for alertCheck)
   //    where type = ? and address = ?
   //     and time <= ?
   //     and time > ?

   rangeEnd.setField(&rangeEndDef);

   selectSampleInRange = new cDbStatement(tableSamples);

   selectSampleInRange->build("select ");
   selectSampleInRange->bind("ADDRESS", cDBS::bndOut);
   selectSampleInRange->bind("TYPE", cDBS::bndOut, ", ");
   selectSampleInRange->bind("TIME", cDBS::bndOut, ", ");
   selectSampleInRange->bind("VALUE", cDBS::bndOut, ", ");
   selectSampleInRange->build(" from %s where ", tableSamples->TableName());
   selectSampleInRange->bind("ADDRESS", cDBS::bndIn | cDBS::bndSet);
   selectSampleInRange->bind("TYPE", cDBS::bndIn | cDBS::bndSet, " and ");
   selectSampleInRange->bindCmp(0, &rangeEnd, "<=", " and ");
   selectSampleInRange->bindCmp(0, "TIME", 0, ">", " and ");
   selectSampleInRange->build(" order by time");

   status += selectSampleInRange->prepare();

   return status;

}

int P4d::exitDb()
{
   delete tablePellets;               tablePellets= nullptr;
   delete tableMenu;                  tableMenu = nullptr;
   delete tableSensorAlert;           tableSensorAlert = nullptr;
   delete tableErrors;                tableErrors = nullptr;
   delete tableTimeRanges;            tableTimeRanges = nullptr;

   delete selectAllMenuItems;         selectAllMenuItems = nullptr;
   delete selectMenuItemsByParent;    selectMenuItemsByParent = nullptr;
   delete selectMenuItemsByChild;     selectMenuItemsByChild = nullptr;
   delete selectSensorAlerts;         selectSensorAlerts = nullptr;
   delete selectAllSensorAlerts;      selectAllSensorAlerts = nullptr;
   delete selectSampleInRange;        selectSampleInRange = nullptr;
   delete selectAllErrors;            selectAllErrors = nullptr;
   delete selectPendingErrors;        selectPendingErrors = nullptr;

   delete selectAllPellets;           selectAllPellets = nullptr;
   delete selectStokerHours;          selectStokerHours = nullptr;
   delete selectStateDuration;        selectStateDuration = nullptr;

   return Daemon::exitDb();
}

int P4d::loadStates()
{
   Daemon::loadStates();

   return done;
}

//***************************************************************************
// Read Configuration
//***************************************************************************

int P4d::readConfiguration(bool initial)
{
   Daemon::readConfiguration(initial);

   getConfigItem("stateCheckInterval", stateCheckInterval, 10);
   getConfigItem("ttyDevice", ttyDevice, "/dev/ttyUSB0");

   getConfigItem("tsync", tSync, no);
   getConfigItem("maxTimeLeak", maxTimeLeak, 10);
   getConfigItem("stateMailStates", stateMailAtStates, "0,1,3,19");
   getConfigItem("consumptionPerHour", consumptionPerHour, 0);
   getConfigItem("heatingType", heatingType, "P4");
   tell(eloDetail, "The heating type is set to '%s'", heatingType);
   getConfigItem("knownStates", knownStates, "");

   if (!isEmpty(knownStates))
   {
      std::vector<std::string> sStates = split(knownStates, ':');
      for (const auto& s : sStates)
         stateDurations[atoi(s.c_str())] = 0;

      tell(eloAlways, "Loaded (%zu) states [%s]", stateDurations.size(), knownStates);
   }

   return done;
}

int P4d::atMeanwhile()
{
   return done;
}

//***************************************************************************
// IO Interrupt Handler
//***************************************************************************

void ioInterrupt()
{
}

//***************************************************************************
// Apply Configuration Specials
//***************************************************************************

int P4d::applyConfigurationSpecials()
{
   sensors["UD"][udState].record = true;
   sensors["UD"][udMode].record = true;
   sensors["UD"][udTime].record = true;

   return done;
}

//***************************************************************************
// Update Sensors
//***************************************************************************

int P4d::updateSensors()
{
   time_t now = time(0);
   sem->p();

   // check serial connection

   int status = request->check();

   if (status != success)
   {
      serial->close();
      tell(eloAlways, "Error reading serial interface, reopen now");
      serial->open(ttyDevice);

      if (request->check() != success)
      {
         sem->v();
         return fail;
      }
   }

   for (const auto& typeSensorsIt : sensors)
   {
      for (const auto& sensorIt : typeSensorsIt.second)
      {
         int status {success};
         const SensorData* sensor = &sensorIt.second;

         cDbRow* row = valueFactOf(sensor->type.c_str(), sensor->address);

         if (sensor->type == "SD")   // state duration
         {
            const auto it = stateDurations.find(sensor->address);

            if (it == stateDurations.end())
               continue;

            double theValue = stateDurations[sensor->address] / 60;

            if (sensors[sensor->type][sensor->address].value != theValue)
            {
               sensors[sensor->type][sensor->address].value = theValue;
               sensors[sensor->type][sensor->address].valid = true;
               sensors[sensor->type][sensor->address].last = now;
            }
         }
         else if (sensor->type == "UD")
         {
            std::string oldText = sensors[sensor->type][sensor->address].text;
            double oldValue = sensors[sensor->type][sensor->address].value;

            if (sensor->address == udState)
            {
               sensors[sensor->type][sensor->address].value = currentState.state;
               sensors[sensor->type][sensor->address].text = currentState.stateinfo;
            }
            else if (sensor->address == udMode)
            {
               sensors[sensor->type][sensor->address].text = currentState.modeinfo;
               sensors[sensor->type][sensor->address].value = currentState.mode;
            }
            else if (sensor->address == udTime)
            {
               sensors[sensor->type][sensor->address].text = l2pTime(currentState.time, "%A, %d. %b. %Y %H:%M:%S");
               sensors[sensor->type][sensor->address].value = currentState.time;
               break;
            }

            sensors[sensor->type][sensor->address].valid = true;

            if (sensors[sensor->type][sensor->address].text != oldText || sensors[sensor->type][sensor->address].value != oldValue)
               sensors[sensor->type][sensor->address].last = now;
         }
         else if (sensor->type == "DO")
         {
            Fs::IoValue v(sensor->address);

            if ((status = request->getDigitalOut(&v)) != success)
            {
               tell(eloAlways, "Error: Getting digital out 0x%04x failed, error %d", sensor->address, status);
               continue;
            }

            if (sensors[sensor->type][sensor->address].state != (bool)v.state)
            {
               sensors[sensor->type][sensor->address].state = v.state;
               sensors[sensor->type][sensor->address].valid = true;
               sensors[sensor->type][sensor->address].last = now;
            }
         }
         else if (sensor->type == "DI")
         {
            Fs::IoValue v(sensor->address);

            if ((status = request->getDigitalIn(&v)) != success)
            {
               tell(eloAlways, "Error: Getting digital in 0x%04x failed, error %d", sensor->address, status);
               continue;
            }

            if (sensors[sensor->type][sensor->address].state != (bool)v.state)
            {
               sensors[sensor->type][sensor->address].state = v.state;
               sensors[sensor->type][sensor->address].valid = true;
               sensors[sensor->type][sensor->address].last = now;
            }
         }
         else if (sensor->type == "AO")
         {
            Fs::IoValue v(sensor->address);

            if ((status = request->getAnalogOut(&v)) != success)
            {
               tell(eloAlways, "Error: Getting analog out 0x%04x failed, error %d", sensor->address, status);
               continue;
            }

            if (sensors[sensor->type][sensor->address].state != (bool)v.state)
            {
               sensors[sensor->type][sensor->address].value = v.state;
               sensors[sensor->type][sensor->address].valid = true;
               sensors[sensor->type][sensor->address].last = now;
            }
         }
         else if (sensor->type == "VA")
         {
            Value v(sensor->address);

            if ((status = request->getValue(&v)) != success)
            {
               tell(eloAlways, "Error: Getting value 0x%04x failed, error %d", sensor->address, status);
               continue;
            }

            int dataType = row->getIntValue("SUBTYPE");
            int value = dataType == 1 ? (word)v.value : (sword)v.value;
            double theValue = value / (double)sensor->factor;

            if (sensors[sensor->type][sensor->address].value != theValue)
            {
               sensors[sensor->type][sensor->address].kind = "value";
               sensors[sensor->type][sensor->address].value = theValue;
               sensors[sensor->type][sensor->address].valid = true;
               sensors[sensor->type][sensor->address].last = now;
            }
         }

         // publish to HA always - we like to draw charts ...

         mqttHaPublish(sensors[sensor->type][sensor->address]);

         if (sensors[sensor->type][sensor->address].last == now || sensor->type == "UD")
            mqttNodeRedPublishSensor(sensors[sensor->type][sensor->address]);
      }
   }

   sem->v();
   selectActiveValueFacts->freeResult();

   return done;
}

int P4d::standbyUntil()
{
   time_t until = min(nextStateAt, nextRefreshAt);

   while (time(0) < until && !doShutDown())
   {
      meanwhile();
      usleep(5000);
   }

   return done;
}

//***************************************************************************
// Do State
//***************************************************************************

int P4d::doLoop()
{
   int lastState {currentState.state};
   int status = updateState();

   if (status != success)
   {
      sem->p();
      serial->close();
      tell(eloAlways, "Error reading serial interface, reopen now!");
      status = serial->open(ttyDevice);
      sem->v();

      if (status != success)
      {
         tell(eloAlways, "Retrying in 10 seconds");
         standby(10);
      }

      return fail;
   }

   stateChanged = lastState != currentState.state;

   if (stateChanged)
   {
      lastState = currentState.state;
      nextRefreshAt = time(0);              // force on state change
      tell(eloAlways, "State changed to '%s'", currentState.stateinfo);
   }

   nextStateAt = stateCheckInterval ? time(0) + stateCheckInterval : nextRefreshAt;

   return success;
}

//***************************************************************************
// Update State
//***************************************************************************

int P4d::updateState()
{
   static time_t nextReportAt = 0;

   int status;
   time_t now;

   // get state

   sem->p();
   tell(eloDetail, "Checking state ...");
   status = request->getStatus(&currentState);
   now = time(0);
   sem->v();

   if (status != success)
      return status;

   tell(eloDetail, "... got (%d) '%s'%s", currentState.state, toTitle(currentState.state),
        isError(currentState.state) ? " -> Störung" : "");

   // ----------------------
   // check time sync

   if (!nextTimeSyncAt)
      scheduleTimeSyncIn();

   if (tSync && maxTimeLeak && labs(currentState.time - now) > maxTimeLeak)
   {
      if (now > nextReportAt)
      {
         tell(eloAlways, "Time drift is %ld seconds", currentState.time - now);
         nextReportAt = now + 2 * tmeSecondsPerMinute;
      }

      if (now > nextTimeSyncAt)
      {
         scheduleTimeSyncIn(tmeSecondsPerDay);

         tell(eloAlways, "Time drift is %ld seconds, syncing now", currentState.time - now);

         sem->p();

         if (request->syncTime() == success)
            tell(eloAlways, "Time sync succeeded");
         else
            tell(eloAlways, "Time sync failed");

         sleep(2);   // S-3200 need some seconds to store time :o

         status = request->getStatus(&currentState);
         now = time(0);

         sem->v();

         tell(eloAlways, "Time drift now %ld seconds", currentState.time - now);
      }
   }

   sensors["UD"][udState].value = currentState.state;
   sensors["UD"][udState].text = currentState.stateinfo;

   mqttHaPublish(sensors["UD"][udState]);
   mqttNodeRedPublishSensor(sensors["UD"][udState]);

   // #TODO -> push also to WS ?

   return status;
}

//***************************************************************************
// Schedule Time Sync In
//***************************************************************************

void P4d::scheduleTimeSyncIn(int offset)
{
   struct tm tm = {0};
   time_t now;

   now = time(0);
   localtime_r(&now, &tm);

   tm.tm_sec = 0;
   tm.tm_min = 0;
   tm.tm_hour = 3;
   tm.tm_isdst = -1;               // force DST auto detect

   nextTimeSyncAt = mktime(&tm);
   nextTimeSyncAt += offset;
}

//***************************************************************************
// Send State Mail
//***************************************************************************

int P4d::sendStateMail()
{
   std::string subject = "Heizung - Status: " + std::string(currentState.stateinfo);
   std::string mailBodyHtml;

   for (const auto& typeSensorsIt : sensors)
   {
      for (const auto& sensorIt : typeSensorsIt.second)
      {
         const SensorData* sensor = &sensorIt.second;

         if (sensor->text.length())
            mailBodyHtml += "        <tr><td>" + sensor->title + "</td><td>" + sensor->text + "</td></tr>\n";
         else if (sensor->kind == "status")
            mailBodyHtml += "        <tr><td>" + sensor->title + "</td><td>" + std::string(sensor->state ? "on" : "off") + "</td></tr>\n";
         else
         {
            char value[100];
            sprintf(value, "%.2f", sensor->value);
            mailBodyHtml += "        <tr><td>" + sensor->title + "</td><td>" + std::string(value) + "</td></tr>\n";
         }
      }
   }

   // check

   if (!isMailState() || isEmpty(mailScript) || !mailBodyHtml.length() || isEmpty(stateMailTo))
      return done;

   // HTML mail

   char* html {nullptr};

   loadHtmlHeader();

   asprintf(&html,
            "<html>\n"
            " %s\n"
            "  <body>\n"
            "   <font face=\"Arial\"><br/>WEB Interface: <a href=\"%s\">S 3200</a><br/></font>\n"
            "   <br/>\n"
            "   <table>\n"
            "     <thead>\n"
            "       <tr class=\"head\">\n"
            "         <th><font>Parameter</font></th>\n"
            "         <th><font>Wert</font></th>\n"
            "       </tr>\n"
            "     </thead>\n"
            "     <tbody>\n"
            "%s"
            "     </tbody>\n"
            "   </table>\n"
            "   <br/>\n"
            "  </body>\n"
            "</html>\n",
            htmlHeader.c_str(), webUrl, mailBodyHtml.c_str());

   int result = sendMail(stateMailTo, subject.c_str(), html, "text/html");
   free(html);
   mailBodyHtml = "";

   return result;
}

//***************************************************************************
// Is Mail State
//***************************************************************************

int P4d::isMailState()
{
   int result = no;
   char* mailStates = 0;

   if (isEmpty(stateMailAtStates))
      return yes;

   mailStates = strdup(stateMailAtStates);

   for (const char* p = strtok(mailStates, ","); p; p = strtok(0, ","))
   {
      if (atoi(p) == currentState.state)
      {
         result = yes;
         break;
      }
   }

   free(mailStates);

   return result;
}

//***************************************************************************
// Perform Jobs
//***************************************************************************

int P4d::performJobs()
{
   return done;
}

//***************************************************************************
// After Update
//***************************************************************************

void P4d::afterUpdate()
{
   Daemon::afterUpdate();

   updateErrors();
   calcStateDuration();
   sensorAlertCheck(lastSampleTime);

   if (mail && errorsPending)
      sendErrorMail();

   if (mail && stateChanged)
      sendStateMail();

   json_t* oJson = json_object();
   s3200State2Json(oJson);
   pushOutMessage(oJson, "s3200-state");
}

//***************************************************************************
// Process
//***************************************************************************

int P4d::process()
{
   return success;
}

//***************************************************************************
// Report Actual State
//***************************************************************************

void P4d::logReport()
{
}

//***************************************************************************
// Initialize Menu Structure
//***************************************************************************

int P4d::initMenu(bool updateParameters)
{
   int status;
   Fs::MenuItem m;
   int count = 0;

   // check serial communication

   if (request->check() != success)
   {
      serial->close();
      return fail;
   }

   tableMenu->truncate();
   tableMenu->clear();

   // ...

   for (status = request->getFirstMenuItem(&m); status != Fs::wrnLast && !doShutDown();
        status = request->getNextMenuItem(&m))
   {
      if (status == wrnSkip)
         continue;

      if (status != success)
         break;

      tell(eloDebug, "%3d) Address: 0x%4x, parent: 0x%4x, child: 0x%4x; '%s'",
           count++, m.parent, m.address, m.child, m.description);

      // update table

      tableMenu->clear();

      tableMenu->setValue("STATE", "D");
      tableMenu->setValue("UNIT", m.type == mstAnlOut && isEmpty(m.unit) ? "%" : m.unit);

      tableMenu->setValue("PARENT", m.parent);
      tableMenu->setValue("CHILD", m.child);
      tableMenu->setValue("ADDRESS", m.address);
      tableMenu->setValue("TITLE", m.description);

      tableMenu->setValue("TYPE", m.type);
      tableMenu->setValue("UNKNOWN1", m.unknown1);
      tableMenu->setValue("UNKNOWN2", m.unknown2);

      tableMenu->insert();

      count++;
   }

   tell(eloAlways, "Read %d menu items", count);

   if (updateParameters)
   {
      count = 0;
      tell(eloAlways, "Update menu parameters");
      tableMenu->clear();

      for (int f = selectAllMenuItems->find(); f; f = selectAllMenuItems->fetch())
      {
         updateParameter(tableMenu);
         count++;
      }

      tell(eloAlways, "Updated %d menu parameters", count);
   }

   return success;
}

//***************************************************************************
// Update Parameter
//***************************************************************************

int P4d::updateParameter(cDbTable* tableMenu)
{
   int type = tableMenu->getIntValue("TYPE");
   int paddr = tableMenu->getIntValue("ADDRESS");
   int child = tableMenu->getIntValue("CHILD");

   tell(eloDetail, "Update parameter %d/%d ...", type, paddr);

   sem->p();

   if (type == mstFirmware)
   {
      Fs::Status s;

      if (request->getStatus(&s) == success)
      {
         if (tableMenu->find())
         {
            tableMenu->setValue("VALUE", s.version);
            tableMenu->setValue("UNIT", "");
            tableMenu->update();
         }
      }
   }

   else if (type == mstDigOut || type == mstDigIn || type == mstAnlOut)
   {
      int status;
      Fs::IoValue v(paddr);

      if (type == mstDigOut)
         status = request->getDigitalOut(&v);
      else if (type == mstDigIn)
         status = request->getDigitalIn(&v);
      else
         status = request->getAnalogOut(&v);

      if (status == success)
      {
         char* buf = 0;

         if (type == mstAnlOut)
         {
            if (v.mode == 0xff)
               asprintf(&buf, "%d (A)", v.state);
            else
               asprintf(&buf, "%d (%d)", v.state, v.mode);
         }
         else
            asprintf(&buf, "%s (%c)", v.state ? "on" : "off", v.mode);

         if (tableMenu->find())
         {
            tableMenu->setValue("VALUE", buf);
            tableMenu->setValue("UNIT", "");
            tableMenu->update();
         }

         free(buf);
      }
   }

   else if (type == mstMesswert || type == mstMesswert1)
   {
      int status;
      Fs::Value v(paddr);

      tableValueFacts->clear();
      tableValueFacts->setValue("TYPE", "VA");
      tableValueFacts->setValue("ADDRESS", paddr);

      if (tableValueFacts->find())
      {
         double factor = tableValueFacts->getIntValue("FACTOR");
         const char* unit = tableValueFacts->getStrValue("UNIT");
         int dataType = tableValueFacts->getIntValue("SUBTYPE");

         status = request->getValue(&v);

         if (status == success)
         {
            char* buf = 0;
            int value = dataType == 1 ? (word)v.value : (sword)v.value;

            asprintf(&buf, "%.2f", value / factor);

            if (tableMenu->find())
            {
               tableMenu->setValue("VALUE", buf);
               tableMenu->setValue("UNIT", strcmp(unit, "°") == 0 ? "°C" : unit);
               tableMenu->update();
            }

            free(buf);
         }
      }
   }
   else if (isGroup(type) || type == mstBusValues || type == mstReset || type == mstEmpty)
   {
      // nothing to do
   }
   else if (child)
   {
      // I have childs -> I have no value -> nothing to do
   }
   else if (paddr == 0 && type != mstPar)
   {
      // address 0 only for type mstPar
   }
   else if (paddr == 9997 || paddr == 9998 || paddr == 9999)
   {
      // this 3 'special' addresses takes a long while and don't deliver any usefull data
   }
   else
   {
      Fs::ConfigParameter p(paddr);

      if (request->getParameter(&p) == success)
      {
         cRetBuf value = p.toNice(type);

         if (tableMenu->find())
         {
            tableMenu->setValue("VALUE", value);
            tableMenu->setValue("UNIT", strcmp(p.unit, "°") == 0 ? "°C" : p.unit);
            tableMenu->setValue("DIGITS", p.digits);
            tableMenu->setValue("MIN", p.rMin);
            tableMenu->setValue("MAX", p.rMax);
            tableMenu->setValue("DEF", p.rDefault);
            tableMenu->setValue("FACTOR", p.getFactor());
            tableMenu->setValue("PUB1", p.ub1);
            tableMenu->setValue("PUB2", p.ub2);
            tableMenu->setValue("PUB3", p.ub3);
            tableMenu->setValue("PUW1", p.uw1);
            tableMenu->update();
         }
      }
   }

   sem->v();

   return done;
}

//***************************************************************************
// Update Time Range Data
//***************************************************************************

int P4d::updateTimeRangeData()
{
   Fs::TimeRanges t;
   int status;
   char fName[10+TB];
   char tName[10+TB];

   tell(eloAlways, "Updating time ranges data ...");

   if (request->check() != success)
   {
      tell(eloAlways, "request->check failed");
      serial->close();
      return fail;
   }

   // update / insert time ranges

   for (status = request->getFirstTimeRanges(&t); status == success; status = request->getNextTimeRanges(&t))
   {
      tableTimeRanges->clear();
      tableTimeRanges->setValue("ADDRESS", t.address);

      for (int n = 0; n < 4; n++)
      {
         sprintf(fName, "FROM%d", n+1);
         sprintf(tName, "TO%d", n+1);
         tableTimeRanges->setValue(fName, t.getTimeRangeFrom(n));
         tableTimeRanges->setValue(tName, t.getTimeRangeTo(n));
      }

      tableTimeRanges->store();
      tableTimeRanges->reset();

   }

   tell(eloAlways, "Updating time ranges data done");

   return done;
}

//***************************************************************************
// Update Errors
//***************************************************************************

int P4d::updateErrors()
{
   int status;
   Fs::ErrorInfo e;
   char timeField[5+TB] = "";
   time_t timeOne = 0;
   cTimeMs timeMs;

   cDbStatement* select = new cDbStatement(tableErrors);
   select->build("select ");
   select->bindAllOut();
   select->build(" from %s where ", tableErrors->TableName());
   select->bind("NUMBER", cDBS::bndIn | cDBS::bndSet);
   select->bind("TIME1", cDBS::bndIn | cDBS::bndSet, " and ");

   if (select->prepare() != success)
   {
      tell(eloAlways, "prepare failed!");
      return fail;
   }

   tell(eloInfo, "Updating error list");

   sem->p();

   for (status = request->getFirstError(&e); status == success; status = request->getNextError(&e))
   {
      int insert = yes;

      sprintf(timeField, "TIME%d", e.state);

      tell(eloDebug2, "Debug: S-3200 error-message %d / %d '%s' '%s' %d [%s]; (for %s)",
           e.number, e.state, l2pTime(e.time).c_str(),  Fs::errState2Text(e.state), e.info, e.text,
           timeField);

      if (e.state == 1)
         timeOne = e.time;

      if (!timeOne)
         continue;

      if (timeOne)
      {
         tableErrors->clear();
         tableErrors->setValue("NUMBER", e.number);
         tableErrors->setValue("TIME1", timeOne);

         insert = !select->find();
      }

      tableErrors->clearChanged();

      if (insert
          || (e.state == 2 && !tableErrors->hasValue("STATE", Fs::errState2Text(2)))
          || (e.state == 4 && tableErrors->hasValue("STATE", Fs::errState2Text(1))))
      {
         tableErrors->setValue(timeField, e.time);
         tableErrors->setValue("STATE", Fs::errState2Text(e.state));
         tableErrors->setValue("NUMBER", e.number);
         tableErrors->setValue("INFO", e.info);
         tableErrors->setValue("TEXT", e.text);
      }

      if (insert)
         tableErrors->insert();
      else if (tableErrors->getChanges())
         tableErrors->update();

      if (e.state == 2)
         timeOne = 0;
   }

   sem->v();
   delete select;

   tell(eloInfo, "Updating error list done in %" PRIu64 "ms", timeMs.Elapsed());

   // count pending (not 'quittiert' AND not mailed) errors

   tableErrors->clear();
   selectPendingErrors->find();
   errorsPending = selectPendingErrors->getResultCount();
   selectPendingErrors->freeResult();

   tell(eloDetail, "Info: Found (%d) pending errors", errorsPending);

   return success;
}

//***************************************************************************
// Send Error Mail
//***************************************************************************

int P4d::sendErrorMail()
{
   std::string body = "";
   const char* subject = "Heizung: STÖRUNG";
   char* html = 0;

   // check

   if (isEmpty(mailScript) || isEmpty(errorMailTo))
      return done;

   // build mail ..

   for (int f = selectPendingErrors->find(); f; f = selectPendingErrors->fetch())
   {
      char* line = 0;
      time_t t = std::max(std::max(tableErrors->getTimeValue("TIME1"), tableErrors->getTimeValue("TIME4")), tableErrors->getTimeValue("TIME2"));

      asprintf(&line, "        <tr><td>%s</td><td>%s</td><td>%s</td></tr>\n",
               l2pTime(t).c_str(), tableErrors->getStrValue("TEXT"), tableErrors->getStrValue("STATE"));

      body += line;

      tell(eloDebug, "Debug: MAILCNT is (%ld), setting to (%ld)",
           tableErrors->getIntValue("MAILCNT"), tableErrors->getIntValue("MAILCNT")+1);
      tableErrors->find();
      tableErrors->setValue("MAILCNT", tableErrors->getIntValue("MAILCNT")+1);
      tableErrors->update();

      free(line);
   }

   selectPendingErrors->freeResult();

   // HTML mail

   loadHtmlHeader();

   asprintf(&html,
            "<html>\n"
            " %s\n"
            "  <body>\n"
            "   <font face=\"Arial\"><br/>WEB Interface: <a href=\"%s\">S 3200</a><br/></font>\n"
            "   <br/>\n"
            "Aktueller Status: %s"
            "   <br/>\n"
            "   <table>\n"
            "     <thead>\n"
            "       <tr class=\"head\">\n"
            "         <th><font>Zeit</font></th>\n"
            "         <th><font>Fehler</font></th>\n"
            "         <th><font>Status</font></th>\n"
            "       </tr>\n"
            "     </thead>\n"
            "     <tbody>\n"
            "%s"
            "     </tbody>\n"
            "   </table>\n"
            "   <br/>\n"
            "  </body>\n"
            "</html>\n",
            htmlHeader.c_str(), webUrl, currentState.stateinfo, body.c_str());

   int result = sendMail(errorMailTo, subject, html, "text/html");

   free(html);

   return result;
}

//***************************************************************************
// Calc State Duration
//***************************************************************************

int P4d::calcStateDuration()
{
   time_t beginTime {0};
   int thisState = {-1};
   std::string text {""};
   std::string kStates {""};

   for (auto& s : stateDurations)
   {
      s.second = 0;
      kStates += ":" + std::to_string(s.first);
   }

   if (knownStates != kStates)
   {
      setConfigItem("knownStates", kStates.c_str());
      getConfigItem("knownStates", knownStates, "");
   }

   tableSamples->clear();
   tableSamples->setValue("TIME", beginTime);
   tableSamples->setValue("VALUE", (double)thisState);

   while (selectStateDuration->find())
   {
      time_t eTime {time(0)};

      if (!endTime.isNull())
         eTime = endTime.getTimeValue();

      if (beginTime)
      {
         stateDurations[thisState] += eTime-beginTime;
         tell(eloDetail, "%s:0x%02x (%s) '%d/%s' %.2f minutes", "SD", thisState,
              l2pTime(beginTime).c_str(), thisState, text.c_str(), (eTime-beginTime) / 60.0);
      }

      if (endTime.isNull())
         break;

      thisState = tableSamples->getFloatValue("VALUE");
      text = tableSamples->getStrValue("TEXT");
      beginTime = eTime;

      addValueFact(thisState, "SD", 1, ("State_Duration_"+std::to_string(thisState)).c_str(),
                   "min", (std::string(text)+" (Laufzeit/Tag)").c_str());

      selectStateDuration->freeResult();
      tableSamples->clear();
      tableSamples->setValue("TIME", beginTime);
      tableSamples->setValue("VALUE", (double)thisState);
   }

   selectStateDuration->freeResult();

   if (eloquence % eloDetail)
   {
      int total {0};

      for (const auto& d : stateDurations)
      {
         tell(eloDebug, "%d: %ld minutes", d.first, d.second / 60);
         total += d.second;
      }

      tell(eloDetail, "total: %d minutes", total / 60);
   }

   return success;
}

//***************************************************************************
// Sensor Alert Check
//***************************************************************************

void P4d::sensorAlertCheck(time_t now)
{
   tableSensorAlert->clear();
   tableSensorAlert->setValue("KIND", "M");

   // iterate over all alert roules ..

   for (int f = selectSensorAlerts->find(); f; f = selectSensorAlerts->fetch())
   {
      alertMailBody = "";
      alertMailSubject = "";
      performAlertCheck(tableSensorAlert->getRow(), now, 0);
   }

   selectSensorAlerts->freeResult();
}

//***************************************************************************
// Perform Login
//***************************************************************************

int P4d::performLogin(json_t* oObject)
{
   Daemon::performLogin(oObject);

   long client = getLongFromJson(oObject, "client");

   json_t* oJson = json_object();
   s3200State2Json(oJson);
   pushOutMessage(oJson, "s3200-state", client);

   return done;
}

//***************************************************************************
// Perform Alert Check
//***************************************************************************

int P4d::performAlertCheck(cDbRow* alertRow, time_t now, int recurse, int force)
{
   int alert = 0;

   // data from alert row

   int addr = alertRow->getIntValue("ADDRESS");
   const char* type = alertRow->getStrValue("TYPE");

   int id = alertRow->getIntValue("ID");
   int lgop = alertRow->getIntValue("LGOP");
   time_t lastAlert = alertRow->getIntValue("LASTALERT");
   int maxRepeat = alertRow->getIntValue("MAXREPEAT");

   int minIsNull = alertRow->getValue("MIN")->isNull();
   int maxIsNull = alertRow->getValue("MAX")->isNull();
   int min = alertRow->getIntValue("MIN");
   int max = alertRow->getIntValue("MAX");

   int range = alertRow->getIntValue("RANGEM");
   int delta = alertRow->getIntValue("DELTA");

   // lookup value facts

   tableValueFacts->clear();
   tableValueFacts->setValue("ADDRESS", addr);
   tableValueFacts->setValue("TYPE", type);

   // lookup samples

   tableSamples->clear();
   tableSamples->setValue("ADDRESS", addr);
   tableSamples->setValue("TYPE", type);
   tableSamples->setValue("AGGREGATE", "S");
   tableSamples->setValue("TIME", now);

   if (!tableSamples->find() || !tableValueFacts->find())
   {
      tell(eloAlways, "Info: Can't perform sensor check for %s/%d '%s'", type, addr, l2pTime(now).c_str());
      return 0;
   }

   // data from samples and value facts

   double value = tableSamples->getFloatValue("VALUE");

   const char* title = tableValueFacts->getStrValue("TITLE");
   const char* unit = tableValueFacts->getStrValue("UNIT");

   // -------------------------------
   // check min / max threshold

   if (!minIsNull || !maxIsNull)
   {
      if (force || (!minIsNull && value < min) || (!maxIsNull && value > max))
      {
         tell(eloAlways, "%d) Alert for sensor %s/0x%x, value %.2f not in range (%d - %d)",
              id, type, addr, value, min, max);

         // max one alert mail per maxRepeat [minutes]

         if (force || !lastAlert || lastAlert < time(0)- maxRepeat * tmeSecondsPerMinute)
         {
            alert = 1;
            add2AlertMail(alertRow, title, value, unit);
         }
      }
   }

   // -------------------------------
   // check range delta

   if (range && delta)
   {
      // select value of this sensor around 'time = (now - range)'

      time_t rangeStartAt = time(0) - range*tmeSecondsPerMinute;
      time_t rangeEndAt = rangeStartAt + interval;

      tableSamples->clear();
      tableSamples->setValue("ADDRESS", addr);
      tableSamples->setValue("TYPE", type);
      tableSamples->setValue("AGGREGATE", "S");
      tableSamples->setValue("TIME", rangeStartAt);
      rangeEnd.setValue(rangeEndAt);

      if (selectSampleInRange->find())
      {
         double oldValue = tableSamples->getFloatValue("VALUE");

         if (force || labs(value - oldValue) > delta)
         {
            tell(eloAlways, "%d) Alert for sensor %s/0x%x , value %.2f changed more than %d in %d minutes",
                 id, type, addr, value, delta, range);

            // max one alert mail per maxRepeat [minutes]

            if (force || !lastAlert || lastAlert < time(0)- maxRepeat * tmeSecondsPerMinute)
            {
               alert = 1;
               add2AlertMail(alertRow, title, value, unit);
            }
         }
      }

      selectSampleInRange->freeResult();
      tableSamples->reset();
   }

   // ---------------------------
   // Check sub rules recursive

   if (alertRow->getIntValue("SUBID") > 0)
   {
      if (recurse > 50)
      {
         tell(eloAlways, "Info: Aborting recursion after 50 steps, seems to be a config error!");
      }
      else
      {
         tableSensorAlert->clear();
         tableSensorAlert->setValue("ID", alertRow->getIntValue("SUBID"));

         if (tableSensorAlert->find())
         {
            int sAlert = performAlertCheck(tableSensorAlert->getRow(), now, recurse+1);

            switch (lgop)
            {
               case loAnd:    alert = alert &&  sAlert; break;
               case loOr:     alert = alert ||  sAlert; break;
               case loAndNot: alert = alert && !sAlert; break;
               case loOrNot:  alert = alert || !sAlert; break;
            }
         }
      }
   }

   // ---------------------------------
   // update master row and send mail

   if (alert && !recurse)
   {
      tableSensorAlert->clear();
      tableSensorAlert->setValue("ID", id);

      if (tableSensorAlert->find())
      {
         if (!force)
         {
            tableSensorAlert->setValue("LASTALERT", time(0));
            tableSensorAlert->update();
         }

         sendAlertMail(tableSensorAlert->getStrValue("MADDRESS"));
      }
   }

   return alert;
}

//***************************************************************************
// Send Mail
//***************************************************************************

int P4d::sendAlertMail(const char* to)
{
   // check

   if (isEmpty(to) || isEmpty(mailScript))
      return done;

   if (alertMailBody.empty())
      alertMailBody = "- undefined -";

   char* html = 0;

   alertMailBody = strReplace("\n", "<br/>\n", alertMailBody);

   const char* htmlHead =
      "<head>\n"
      "  <style type=\"text/css\">\n"
      "    caption { background: #095BA6; font-family: Arial Narrow; color: #fff; font-size: 18px; }\n"
      "  </style>\n"
      "</head>\n";

   asprintf(&html,
            "<html>\n"
            " %s"
            " <body>\n"
            "  %s\n"
            " </body>\n"
            "</html>\n",
            htmlHead, alertMailBody.c_str());

   alertMailBody = html;
   free(html);

   // send mail

   return sendMail(to, alertMailSubject.c_str(), alertMailBody.c_str(), "text/html");
}

//***************************************************************************
// Send Mail
//***************************************************************************

int P4d::add2AlertMail(cDbRow* alertRow, const char* title, double value, const char* unit)
{
   char* sensor = 0;

   std::string subject = alertRow->getStrValue("MSUBJECT");
   std::string body = alertRow->getStrValue("MBODY");
   int addr = alertRow->getIntValue("ADDRESS");
   const char* type = alertRow->getStrValue("TYPE");

   int min = alertRow->getIntValue("MIN");
   int max = alertRow->getIntValue("MAX");
   int range = alertRow->getIntValue("RANGEM");
   int delta = alertRow->getIntValue("DELTA");
   int maxRepeat = alertRow->getIntValue("MAXREPEAT");
   double minv {0};
   double maxv {0};

   tablePeaks->clear();
   tablePeaks->setValue("ADDRESS", alertRow->getIntValue("ADDRESS"));
   tablePeaks->setValue("TYPE", alertRow->getStrValue("TYPE"));

   if (tablePeaks->find())
   {
      minv = tablePeaks->getFloatValue("MIN");
      maxv = tablePeaks->getFloatValue("MAX");
   }

   tablePeaks->reset();

   if (!body.length())
      body = "- undefined -";

   // prepare

   asprintf(&sensor, "%s/0x%x", type, addr);

   // templating

   subject = strReplace("%sensorid%", sensor, subject);
   subject = strReplace("%value%", value, subject);
   subject = strReplace("%unit%", unit, subject);
   subject = strReplace("%title%", title, subject);
   subject = strReplace("%min%", (long)min, subject);
   subject = strReplace("%max%", (long)max, subject);
   subject = strReplace("%range%", (long)range, subject);
   subject = strReplace("%delta%", (long)delta, subject);
   subject = strReplace("%time%", l2pTime(time(0)).c_str(), subject);
   subject = strReplace("%repeat%", (long)maxRepeat, subject);
   subject = strReplace("%weburl%", webUrl, subject);
   subject = strReplace("%minv%", minv, subject);
   subject = strReplace("%maxv%", maxv, subject);

   body = strReplace("%sensorid%", sensor, body);
   body = strReplace("%value%", value, body);
   body = strReplace("%unit%", unit, body);
   body = strReplace("%title%", title, body);
   body = strReplace("%min%", (long)min, body);
   body = strReplace("%max%", (long)max, body);
   body = strReplace("%range%", (long)range, body);
   body = strReplace("%delta%", (long)delta, body);
   body = strReplace("%time%", l2pTime(time(0)).c_str(), body);
   body = strReplace("%repeat%", (long)maxRepeat, body);
   body = strReplace("%weburl%", webUrl, body);
   body = strReplace("%minv%", minv, body);
   body = strReplace("%maxv%", maxv, body);

   alertMailSubject += std::string(" ") + subject;
   alertMailBody += std::string("\n") + body;

   free(sensor);

   return success;
}

//***************************************************************************
// Dispatch Special Request
//***************************************************************************

int P4d::dispatchSpecialRequest(Event event, json_t* oObject, long client)
{
   int status {fail};

   switch (event)
   {
      case evInitTables:        status = performInitTables(oObject, client);       break;
      case evUpdateTimeRanges:  status = performUpdateTimeRanges(oObject, client); break;
      case evPellets:           status = performPellets(oObject, client);          break;
      case evPelletsAdd:        status = performPelletsAdd(oObject, client);       break;
      case evStoreAlerts:       status = storeAlerts(oObject, client);             break;
      case evParEditRequest:    status = performParEditRequest(oObject, client);   break;
      case evParStore:          status = performParStore(oObject, client);         break;
      case evMenu:              status = performMenu(oObject, client);             break;
      case evAlerts:            status = performAlerts(oObject, client);           break;
      case evErrors:            status = performErrors(client);                    break;
      default:                  return ignore;
   }

   return status;
}

//***************************************************************************
// Check Rights
//***************************************************************************

bool P4d::onCheckRights(long client, Event event, uint rights)
{
   switch (event)
   {
      case evMenu:                return rights & urView;
      case evParEditRequest:      return rights & urFullControl;
      case evParStore:            return rights & urFullControl;
      case evInitTables:          return rights & urSettings;
      case evUpdateTimeRanges:    return rights & urFullControl;
      case evPellets:             return rights & urControl;
      case evPelletsAdd:          return rights & urFullControl;
      case evErrors:              return rights & urView;
      default: break;
   }

   return false;
}

//***************************************************************************
// Perform Init Tables
//***************************************************************************

int P4d::performInitTables(json_t* oObject, long client)
{
   const char* action = getStringFromJson(oObject, "action");

   if (isEmpty(action))
      return replyResult(fail, "missing action", client);

   if (strcmp(action, "valuefacts") == 0)
   {
      initValueFacts();
      updateTimeRangeData();
   }
   else if (strcmp(action, "menu") == 0)
   {
      initMenu();
   }
   else if (strcmp(action, "menu-force") == 0)
   {
      initMenu(true);
   }

   return replyResult(success, "... init abgeschlossen!", client);
}

//***************************************************************************
// Perform Pellets Request
//***************************************************************************

int P4d::performPellets(json_t* oObject, long client)
{
   uint stokerHhLast {0};
   time_t timeLast {0};
   double tAmount {0.0};
   double tPrice {0.0};
   json_t* oJson = json_array();
   double consumptionHLast {0.0};

   tablePellets->clear();

   for (int f = selectAllPellets->find(); f; f = selectAllPellets->fetch())
   {
      json_t* oData = json_object();
      json_array_append_new(oJson, oData);

      json_object_set_new(oData, "id", json_integer(tablePellets->getIntValue("ID")));
      json_object_set_new(oData, "time", json_integer(tablePellets->getTimeValue("TIME")));
      json_object_set_new(oData, "amount", json_integer(tablePellets->getIntValue("AMOUNT")));
      json_object_set_new(oData, "price", json_real(tablePellets->getFloatValue("PRICE")));
      json_object_set_new(oData, "comment", json_string(tablePellets->getStrValue("COMMENT")));

      double amount = tablePellets->getIntValue("AMOUNT");
      tAmount += tablePellets->getIntValue("AMOUNT");
      tPrice += tablePellets->getFloatValue("PRICE");

      // ...

      minValue.clear();
      tableSamples->clear();
      tableSamples->setValue("TYPE", "VA");
      tableSamples->setValue("ADDRESS", 0xad);
      tableSamples->setValue("TIME", tablePellets->getTimeValue("TIME"));

      if (!selectStokerHours->find())
      {
         tell(eloAlways, "Info: Sample for stoker hours not found!");
         continue;
      }

      if (timeLast)
      {
         uint durationDays = (tablePellets->getTimeValue("TIME") - timeLast) / tmeSecondsPerDay;
         json_object_set_new(oData, "duration", json_integer(durationDays));
      }

      uint stokerHh = minValue.getIntValue();   // is bound as int !!

      tell(eloAlways, "Getankt: '%s' stokerH %d at '%s'",
           l2pTime(tablePellets->getTimeValue("TIME")).c_str(),
           stokerHh, l2pTime(tableSamples->getTimeValue("TIME")).c_str());

      if (stokerHhLast && stokerHh-stokerHhLast > 0)
      {
         tell(eloAlways, "stokerHh delta => %d", stokerHh - stokerHhLast);
         json_object_set_new(oData, "stokerHours", json_integer(stokerHh - stokerHhLast));
         consumptionHLast = amount/(stokerHh-stokerHhLast);
         json_object_set_new(oData, "consumptionH", json_real(consumptionHLast));
      }

      timeLast = tablePellets->getTimeValue("TIME");
      stokerHhLast = stokerHh;
   }

   selectAllPellets->freeResult();

   double consumptionH = consumptionPerHour ? consumptionPerHour : consumptionHLast;
   char* hint;
   asprintf(&hint, "consumption sice tankering by %.2f kg / stoker hour", consumptionH);
   tell(eloAlways, "Calculating %s", hint);

   json_t* oData = json_object();
   json_array_append_new(oJson, oData);

   json_object_set_new(oData, "id", json_integer(-1));
   json_object_set_new(oData, "sum", json_boolean(true));
   json_object_set_new(oData, "time", json_integer(time(0)));
   json_object_set_new(oData, "amount", json_integer(tAmount));
   json_object_set_new(oData, "price", json_real(tPrice));
   json_object_set_new(oData, "comment", json_string("Total"));
   json_object_set_new(oData, "consumptionHint", json_string(hint));
   json_object_set_new(oData, "stokerHours", json_integer(sensors["VA"][0xad].value - stokerHhLast));
   json_object_set_new(oData, "consumptionDelta", json_integer(consumptionH * (sensors["VA"][0xad].value-stokerHhLast)));

   free(hint);

   return pushOutMessage(oJson, "pellets", client);
}

//***************************************************************************
// Perform Pellets Add Entry
//***************************************************************************

int P4d::performPelletsAdd(json_t* oObject, long client)
{
   int id = getIntFromJson(oObject, "id", -1);
   bool del = getBoolFromJson(oObject, "delete", false);

   tablePellets->clear();
   tablePellets->setValue("ID", id);

   if (del)
   {
      if (!tablePellets->find())
         return replyResult(success, "failed", client);

      tablePellets->deleteWhere("id = %d", id);
      performPellets(0, client);
      return replyResult(success, "Eintrag gelöscht", client);
   }

   if (id >= 0 && !tablePellets->find())
      return replyResult(success, "failed", client);

   tablePellets->setValue("TIME", getLongFromJson(oObject, "time"));
   tablePellets->setValue("AMOUNT", getIntFromJson(oObject, "amount"));
   tablePellets->setValue("PRICE", getDoubleFromJson(oObject, "price"));
   tablePellets->setValue("COMMENT", getStringFromJson(oObject, "comment"));
   tablePellets->store();

   performPellets(0, client);

   return replyResult(success, "Eintrag gespeichert", client);
}

//***************************************************************************
// Perform WS Error Data Request
//***************************************************************************

int P4d::performErrors(long client)
{
   if (client == 0)
      return done;

   json_t* oJson = json_array();

   tableErrors->clear();

   for (int f = selectAllErrors->find(); f; f = selectAllErrors->fetch())
   {
      json_t* oData = json_object();
      json_array_append_new(oJson, oData);

      time_t t = std::max(std::max(tableErrors->getTimeValue("TIME1"), tableErrors->getTimeValue("TIME4")), tableErrors->getTimeValue("TIME2"));
      std::string strTime = l2pTime(t);
      uint duration {0};

      if (tableErrors->getValue("TIME2")->isNull())
         duration = time(0) - tableErrors->getTimeValue("TIME1");
      else
         duration = tableErrors->getTimeValue("TIME2") - tableErrors->getTimeValue("TIME1");

      json_object_set_new(oData, "state", json_string(tableErrors->getStrValue("STATE")));
      json_object_set_new(oData, "text", json_string(tableErrors->getStrValue("TEXT")));
      json_object_set_new(oData, "duration", json_integer(duration));
      json_object_set_new(oData, "time", json_string(strTime.c_str()));
   }

   selectAllErrors->freeResult();

   pushOutMessage(oJson, "errors", client);

   return done;
}
int P4d::performAlertTestMail(int id, long client)
{
   tell(eloDetail, "Test mail for alert (%d) requested", id);

   if (isEmpty(mailScript))
      return replyResult(fail, "missing mail script", client);

   if (!fileExists(mailScript))
      return replyResult(fail, "mail script not found", client);

   if (!selectMaxTime->find())
      tell(eloAlways, "Warning: Got no result by 'select max(time) from samples'");

   time_t  last = tableSamples->getTimeValue("TIME");
   selectMaxTime->freeResult();

   tableSensorAlert->clear();
   tableSensorAlert->setValue("ID", id);

   if (!tableSensorAlert->find())
      return replyResult(fail, "requested alert ID not found", client);

   alertMailBody = "";
   alertMailSubject = "";

   if (!performAlertCheck(tableSensorAlert->getRow(), last, 0, yes/*force*/))
      return replyResult(fail, "send failed", client);

   tableSensorAlert->reset();

   return replyResult(success, "mail sended", client);
}

//***************************************************************************
// Perform WS Parameter Edit Request
//***************************************************************************

int P4d::performParEditRequest(json_t* oObject, long client)
{
   if (client == 0)
      return done;

   int id = getIntFromJson(oObject, "id", na);
   int parent = getIntFromJson(oObject, "parent", 1);

   if (id == 0)
      return performTimeParEditRequest(oObject, client);

   tableMenu->clear();
   tableMenu->setValue("ID", id);

   if (!tableMenu->find())
   {
      tell(eloAlways, "Info: Id %d for 'pareditrequest' not found!", id);
      return fail;
   }

   int type = tableMenu->getIntValue("TYPE");
   unsigned int address = tableMenu->getIntValue("ADDRESS");
   const char* title = tableMenu->getStrValue("TITLE");

   tableMenu->reset();
   sem->p();

   ConfigParameter p(address);

   if (request->getParameter(&p) == success)
   {
      cRetBuf value = p.toNice(type);

      json_t* oJson = json_object();
      json_object_set_new(oJson, "id", json_integer(id));
      json_object_set_new(oJson, "type", json_integer(type));
      json_object_set_new(oJson, "address", json_integer(address));
      json_object_set_new(oJson, "title", json_string(title));
      json_object_set_new(oJson, "unit", json_string(type == mstParZeit ? "Uhr" : p.unit));
      json_object_set_new(oJson, "value", json_string(value));
      json_object_set_new(oJson, "def", json_integer(p.rDefault));
      json_object_set_new(oJson, "min", json_integer(p.rMin));
      json_object_set_new(oJson, "max", json_integer(p.rMax));
      json_object_set_new(oJson, "digits", json_integer(p.digits));
      json_object_set_new(oJson, "parent", json_integer(parent));

      pushOutMessage(oJson, "pareditrequest", client);
   }

   sem->v();

   return done;
}

int P4d::performTimeParEditRequest(json_t* oObject, long client)
{
   if (client == 0)
      return done;

   int trAddr = getIntFromJson(oObject, "address", na);
   int range = getIntFromJson(oObject, "range", na);
   int parent = getIntFromJson(oObject, "parent", 1);

   tableTimeRanges->clear();
   tableTimeRanges->setValue("ADDRESS", trAddr);

   if (!tableTimeRanges->find())
   {
      tell(eloAlways, "Info: Address %d or 'pareditrequest' in timetanges not found!", trAddr);
      return fail;
   }

   char* rTitle {nullptr};
   char* value {nullptr};
   char* from {nullptr};
   char* to {nullptr};

   asprintf(&rTitle, "Range %d", range);
   asprintf(&from, "from%d", range);
   asprintf(&to, "to%d", range);

   if (!tableTimeRanges->hasValue(from, "nn:nn") && !tableTimeRanges->hasValue(to, "nn:nn"))
      asprintf(&value, "%s - %s", tableTimeRanges->getStrValue(from), tableTimeRanges->getStrValue(to));
   else
      asprintf(&value, "%s", "-");

   json_t* oJson = json_object();
   json_object_set_new(oJson, "id", json_integer(0));
   json_object_set_new(oJson, "type", json_integer(mstParZeit));
   json_object_set_new(oJson, "address", json_integer(trAddr));
   json_object_set_new(oJson, "range", json_integer(range));
   json_object_set_new(oJson, "title", json_string(rTitle));
   json_object_set_new(oJson, "unit", json_string("Uhr"));
   json_object_set_new(oJson, "value", json_string(value));
   json_object_set_new(oJson, "parent", json_integer(parent));

   pushOutMessage(oJson, "pareditrequest", client);

   return done;
}

//***************************************************************************
// Perform WS Parameter Store
//***************************************************************************

int P4d::performParStore(json_t* oObject, long client)
{
   if (client == 0)
      return done;

   int id = getIntFromJson(oObject, "id", na);

   if (id == 0)
      return performTimeParStore(oObject, client);

   json_t* oJson = json_object();
   const char* value = getStringFromJson(oObject, "value");

   tableMenu->clear();
   tableMenu->setValue("ID", id);

   if (!tableMenu->find())
   {
      replyResult(fail, "Parameter not found", client);
      tell(eloAlways, "Info: Id %d for 'parstore' not found!", id);
      return done;
   }

   int parent = tableMenu->getIntValue("PARENT");
   int type = tableMenu->getIntValue("TYPE");
   unsigned int address = tableMenu->getIntValue("ADDRESS");
   ConfigParameter p(address);

   sem->p();
   request->getParameter(&p);
   sem->v();

   if (p.setValue(type, value) != success)
   {
      tableMenu->reset();
      tell(eloAlways, "Set of parameter failed, wrong format");
      return replyResult(fail, "Value format error", client);
   }

   {
      int status {fail};
      tell(eloAlways, "Storing value '%s/%s' for parameter at address 0x%x", value, p.toNice(type).string(), address);
      sem->p();

      if ((status = request->setParameter(&p)) == success)
      {
         tableMenu->setValue("VALUE", p.toNice(type));
         tableMenu->setValue("UNIT", p.unit);
         tableMenu->update();
         json_object_set_new(oJson, "parent", json_integer(parent));
         sem->v();

         replyResult(status, "Parameter gespeichert", client);
         return performMenu(oJson, client);
      }

      sem->v();
      tell(eloAlways, "Set of parameter failed, error %d", status);

      if (status == P4Request::wrnNonUpdate)
         replyResult(status, "Value identical, ignoring request", client);
      else if (status == P4Request::wrnOutOfRange)
         replyResult(status, "Value out of range", client);
      else
         replyResult(status, "Serial communication error", client);
   }

   tableMenu->reset();

   return done;
}

int P4d::performTimeParStore(json_t* oObject, long client)
{
   int status {success};
   int trAddr = getIntFromJson(oObject, "address", na);
   int range = getIntFromJson(oObject, "range", na);
   int parent = getIntFromJson(oObject, "parent", 1);
   const char* value = getStringFromJson(oObject, "value");

   tableTimeRanges->clear();
   tableTimeRanges->setValue("ADDRESS", trAddr);

   if (!tableTimeRanges->find() || range < 1)
   {
      replyResult(fail, "Parameter not found", client);
      tell(eloAlways, "Info: Address %d for 'parstore' not found!", trAddr);
      return done;
   }

   Fs::TimeRanges t(trAddr);
   char fName[10+TB];
   char tName[10+TB];
   char valueFrom[100+TB];
   char valueTo[100+TB];

   // parse range and value from data

   if (isEmpty(value))
      value = "nn:nn - nn:nn";

   if (sscanf(value, "%[^-]-%[^-]", valueFrom, valueTo) != 2)
   {
      replyResult(fail, "Parsing of value failed, wrong format!", client);
      tell(eloAlways, "Parsing of value '%s' failed, wrong format!", value);
      return done;
   }

   allTrim(valueFrom);
   allTrim(valueTo);

   // update struct with time ranges from table

   for (int n = 0; n < 4; n++)
   {
      sprintf(fName, "FROM%d", n+1);
      sprintf(tName, "TO%d", n+1);

      status += t.setTimeRange(n, tableTimeRanges->getStrValue(fName), tableTimeRanges->getStrValue(tName));
   }

   // update the changed range with new value

   status += t.setTimeRange(range-1, valueFrom, valueTo);

   if (status != success)
   {
      replyResult(fail, "Set of time range parameter failed, wrong format", client);
      tell(eloAlways, "Set of time range parameter failed, wrong format");
   }

   tell(eloDebug, "Value was: '%s'-'%s'", valueFrom, valueTo);
   tell(eloAlways, "Storing '%s' for time range '%d' of parameter 0x%x", t.getTimeRange(range-1), range, t.address);

   if (request->setTimeRanges(&t) != success)
   {
      replyResult(fail, "Set of time range parameter failed", client);
      tell(eloAlways, "Set of time range parameter failed, error %d", status);
   }

   // update time range table

   sprintf(fName, "FROM%d", range);
   sprintf(tName, "TO%d", range);
   tableTimeRanges->setValue(fName, t.getTimeRangeFrom(range-1));
   tableTimeRanges->setValue(tName, t.getTimeRangeTo(range-1));
   tableTimeRanges->update();

   replyResult(status, "Parameter gespeichert", client);

   json_t* oJson = json_object();
   json_object_set_new(oJson, "parent", json_integer(parent));

   return performMenu(oJson, client);
}

//***************************************************************************
// Perform WS Menu Request
//***************************************************************************

int P4d::performMenu(json_t* oObject, long client)
{
   if (client == 0)
      return done;

   int parent {1};
   int last {0};
   char* title {nullptr};

   if (oObject)
      parent = getIntFromJson(oObject, "parent", 1);

   json_t* oJson = json_object();
   json_t* oArray = json_array();

   tableMenu->clear();
   tableMenu->setValue("CHILD", parent);

   if (selectMenuItemsByChild->find())
   {
      last = tableMenu->getIntValue("PARENT");
      title = strdup(tableMenu->getStrValue("title"));
   }

   selectMenuItemsByChild->freeResult();

   tableMenu->clear();
   tableMenu->setValue("PARENT", parent);

   for (int f = selectMenuItemsByParent->find(); f; f = selectMenuItemsByParent->fetch())
   {
      int type = tableMenu->getIntValue("TYPE");
      int address = tableMenu->getIntValue("ADDRESS");
      int child = tableMenu->getIntValue("CHILD");
      int digits = tableMenu->getIntValue("DIGITS");
      char* title = strdup(tableMenu->getStrValue("TITLE"));

      if (isEmpty(rTrim(title)))
      {
         free(title);
         continue;
      }

      if (title[strlen(title)-1] == ':')
         title[strlen(title)-1] = '\0';

      bool timeGroup = (type == mstMenuChoice || type == mstMenu2) && strcmp(title, "Zeiten") == 0 && (child == 230 || child == 350 || child == 430 || child == 573);

      if (type == mstBusValues || type == mstReset)
         continue;

      // this 3 'special' addresses takes a long while and don't deliver any usefull data

      if (address == 9997 || address == 9998 || address == 9999)
         continue;

      updateParameter(tableMenu);

      if (!child && tableMenu->getValue("VALUE")->isNull())
         continue;

      if (!timeGroup)
      {
         json_t* oData = json_object();
         json_array_append_new(oArray, oData);

         json_object_set_new(oData, "id", json_integer(tableMenu->getIntValue("ID")));
         json_object_set_new(oData, "parent", json_integer(tableMenu->getIntValue("PARENT")));
         json_object_set_new(oData, "child", json_integer(child));
         json_object_set_new(oData, "type", json_integer(type));
         json_object_set_new(oData, "address", json_integer(address));
         json_object_set_new(oData, "title", json_string(title));
         json_object_set_new(oData, "unit", json_string(tableMenu->getStrValue("UNIT")));
         json_object_set_new(oData, "range", json_integer(na));
         json_object_set_new(oData, "parent", json_integer(parent));
         json_object_set_new(oData, "digits", json_integer(digits));

         if (type == mstMesswert || type == mstMesswert1)
            json_object_set_new(oData, "value", json_real(sensors["VA"][address].value));
         else
            json_object_set_new(oData, "value", json_string(tableMenu->getStrValue("VALUE")));

         if (type == mstPar || type == mstParSet || type == mstParSet1 || type == mstParSet2 ||
             type == mstParDig || type == mstParZeit)
            json_object_set_new(oData, "editable", json_boolean(true));
      }

      else
      {
         int baseAddr {0};

         switch (child)
         {
            case 230: baseAddr = 0x00 + (address * 7); break;   // Boiler 'n'
            case 350: baseAddr = 0x38 + (address * 7); break;   // Heizkreis 'n'
            case 430: baseAddr = 0xb6 + (address * 7); break;   // Puffer 'n'
         // case ???: baseAddr = 0xd2 + (address * 7); break;   // Kessel
            case 573: baseAddr = 0xd9 + (address * 7); break;   // Zirkulation
         }

         // updateTimeRangeData();

         for (int wday = 0; wday < 7; wday++)
         {
            int trAddr = baseAddr + wday;
            char* dayTitle {nullptr};
            asprintf(&dayTitle, "Zeiten '%s'", toWeekdayName(wday));

            tableTimeRanges->clear();
            tableTimeRanges->setValue("ADDRESS", trAddr);

            json_t* oData = json_object();
            json_array_append_new(oArray, oData);

            json_object_set_new(oData, "id", json_integer(0));
            json_object_set_new(oData, "parent", json_integer(0));
            json_object_set_new(oData, "child", json_integer(0));
            json_object_set_new(oData, "type", json_integer(0));
            json_object_set_new(oData, "address", json_integer(0));
            json_object_set_new(oData, "title", json_string(dayTitle));
            json_object_set_new(oData, "unit", json_string(""));
            json_object_set_new(oData, "parent", json_integer(parent));

            free(dayTitle);

            if (tableTimeRanges->find())
            {
               for (int n = 1; n < 5; n++)
               {
                  char* rTitle {nullptr};
                  char* value {nullptr};
                  char* from {nullptr};
                  char* to {nullptr};

                  asprintf(&rTitle, "Range %d", n);
                  asprintf(&from, "from%d", n);
                  asprintf(&to, "to%d", n);
                  asprintf(&value, "%s - %s", tableTimeRanges->getStrValue(from), tableTimeRanges->getStrValue(to));

                  json_t* oData = json_object();
                  json_array_append_new(oArray, oData);

                  json_object_set_new(oData, "id", json_integer(0));
                  json_object_set_new(oData, "range", json_integer(n));
                  json_object_set_new(oData, "parent", json_integer(0));
                  json_object_set_new(oData, "child", json_integer(0));
                  json_object_set_new(oData, "type", json_integer(0));
                  json_object_set_new(oData, "address", json_integer(trAddr));
                  json_object_set_new(oData, "title", json_string(rTitle));
                  json_object_set_new(oData, "unit", json_string(""));
                  json_object_set_new(oData, "value", json_string(value));
                  json_object_set_new(oData, "editable", json_boolean(true));
                  json_object_set_new(oData, "parent", json_integer(parent));

                  free(rTitle);
                  free(value);
                  free(from);
                  free(to);
               }

               tableTimeRanges->reset();
            }
         }
      }

      free(title);
   }

   selectMenuItemsByParent->freeResult();

   json_object_set_new(oJson, "items", oArray);
   json_object_set_new(oJson, "parent", json_integer(parent));
   json_object_set_new(oJson, "last", json_integer(last));
   json_object_set_new(oJson, "title", json_string(title));

   pushOutMessage(oJson, "menu", client);

   return done;
}

//***************************************************************************
// Perform WS Sensor Alert Request
//***************************************************************************

int P4d::performAlerts(json_t* oObject, long client)
{
   json_t* oArray = json_array();

   tableSensorAlert->clear();

   for (int f = selectAllSensorAlerts->find(); f; f = selectAllSensorAlerts->fetch())
   {
      json_t* oData = json_object();
      json_array_append_new(oArray, oData);

      json_object_set_new(oData, "id", json_integer(tableSensorAlert->getIntValue("ID")));
      json_object_set_new(oData, "kind", json_string(tableSensorAlert->getStrValue("ID")));
      json_object_set_new(oData, "subid", json_integer(tableSensorAlert->getIntValue("SUBID")));
      json_object_set_new(oData, "lgop", json_integer(tableSensorAlert->getIntValue("LGOP")));
      json_object_set_new(oData, "type", json_string(tableSensorAlert->getStrValue("TYPE")));
      json_object_set_new(oData, "address", json_integer(tableSensorAlert->getIntValue("ADDRESS")));
      json_object_set_new(oData, "state", json_string(tableSensorAlert->getStrValue("STATE")));
      json_object_set_new(oData, "min", json_integer(tableSensorAlert->getIntValue("MIN")));
      json_object_set_new(oData, "max", json_integer(tableSensorAlert->getIntValue("MAX")));
      json_object_set_new(oData, "rangem", json_integer(tableSensorAlert->getIntValue("RANGEM")));
      json_object_set_new(oData, "delta", json_integer(tableSensorAlert->getIntValue("DELTA")));
      json_object_set_new(oData, "maddress", json_string(tableSensorAlert->getStrValue("MADDRESS")));
      json_object_set_new(oData, "msubject", json_string(tableSensorAlert->getStrValue("MSUBJECT")));
      json_object_set_new(oData, "mbody", json_string(tableSensorAlert->getStrValue("MBODY")));
      json_object_set_new(oData, "maxrepeat", json_integer(tableSensorAlert->getIntValue("MAXREPEAT")));

      //json_object_set_new(oData, "lastalert", json_integer(0));
   }

   selectAllSensorAlerts->freeResult();
   pushOutMessage(oArray, "alerts", client);

   return done;
}

//***************************************************************************
// Store Sensor Alerts
//***************************************************************************

int P4d::storeAlerts(json_t* oObject, long client)
{
   const char* action = getStringFromJson(oObject, "action", "");

   if (strcmp(action, "delete") == 0)
   {
      int alertid = getIntFromJson(oObject, "alertid", na);

      tableSensorAlert->deleteWhere("id = %d", alertid);

      performAlerts(0, client);
      replyResult(success, "Sensor Alert gelöscht", client);
   }

   else if (strcmp(action, "store") == 0)
   {
      json_t* array = json_object_get(oObject, "alerts");
      size_t index {0};
      json_t* jObj {nullptr};

      json_array_foreach(array, index, jObj)
      {
         int id = getIntFromJson(jObj, "id", na);

         tableSensorAlert->clear();

         if (id != na)
         {
            tableSensorAlert->setValue("ID", id);

            if (!tableSensorAlert->find())
               continue;
         }

         tableSensorAlert->clearChanged();
         tableSensorAlert->setValue("STATE", getStringFromJson(jObj, "state"));
         tableSensorAlert->setValue("MAXREPEAT", getIntFromJson(jObj, "maxrepeat"));

         tableSensorAlert->setValue("ADDRESS", getIntFromJson(jObj, "address"));
         tableSensorAlert->setValue("TYPE", getStringFromJson(jObj, "type"));
         tableSensorAlert->setValue("MIN", getIntFromJson(jObj, "min"));
         tableSensorAlert->setValue("MAX", getIntFromJson(jObj, "max"));
         tableSensorAlert->setValue("DELTA", getIntFromJson(jObj, "delta"));
         tableSensorAlert->setValue("RANGEM", getIntFromJson(jObj, "rangem"));
         tableSensorAlert->setValue("MADDRESS", getStringFromJson(jObj, "maddress"));
         tableSensorAlert->setValue("MSUBJECT", getStringFromJson(jObj, "msubject"));
         tableSensorAlert->setValue("MBODY", getStringFromJson(jObj, "mbody"));

         if (id == na)
            tableSensorAlert->insert();
         else if (tableSensorAlert->getChanges())
            tableSensorAlert->update();
      }

      performAlerts(0, client);
      replyResult(success, "Konfiguration gespeichert", client);
   }

   return success;
}

//***************************************************************************
// Perform Update TimeRanges
//***************************************************************************

int P4d::performUpdateTimeRanges(json_t* oObject, long client)
{
   int parent = getIntFromJson(oObject, "parent");
   updateTimeRangeData();
   replyResult(success, "... done", client);

   json_t* oJson = json_object();
   json_object_set_new(oJson, "parent", json_integer(parent));

   return performMenu(oJson, client);
}

//***************************************************************************
// Update Value Facts
//***************************************************************************

int P4d::initValueFacts(bool truncate)
{
   int status {success};
   Fs::ValueSpec v;
   int count {0};
   int added {0};
   int modified {0};

   tell(eloAlways, "Update value facts");

   // check serial communication

   if (request->check() != success)
   {
      serial->close();
      return fail;
   }

   if (truncate)
      tableValueFacts->truncate();

   // ---------------------------------
   // Add the sensor definitions delivered by the S 3200

   for (status = request->getFirstValueSpec(&v); status != Fs::wrnLast; status = request->getNextValueSpec(&v))
   {
      if (status != success)
         continue;

      tell(eloDebug, "%3d) 0x%04x '%s' %d '%s' (%04d) '%s'",
           count, v.address, v.name, v.factor, v.unit, v.type, v.description);

      int res = addValueFact(v.address, "VA", v.factor, v.name,
                             strcmp(v.unit, "°") == 0 ? "°C" : v.unit,
                             v.description);

      // set special value SUBTYPE for valuefact

      tableValueFacts->clear();
      tableValueFacts->setValue("ADDRESS", v.address);
      tableValueFacts->setValue("TYPE", "VA");

      tableValueFacts->find();
      tableValueFacts->setValue("SUBTYPE", v.type);
      tableValueFacts->store();

      count++;

      if (res == 1)
         added++;
      else if (res == 2)
         modified++;
   }

   tell(eloAlways, "Read %d value facts, modified %d and added %d", count, modified, added);

   // count = 0;
   // tell(eloAlways, "Update example value of table valuefacts");

   // for (int f = selectAllValueFacts->find(); f; f = selectAllValueFacts->fetch())
   // {
   //    if (!tableValueFacts->hasValue("TYPE", "VA"))
   //       continue;

   //    Value v(tableValueFacts->getIntValue("ADDRESS"));

   //    if ((status = request->getValue(&v)) != success)
   //    {
   //       tell(eloAlways, "Getting value 0x%04x failed, error %d", v.address, status);
   //       continue;
   //    }

   //    double factor = tableValueFacts->getIntValue("FACTOR");
   //    int dataType = tableValueFacts->getIntValue("SUBTYPE");
   //    int value = dataType == 1 ? (word)v.value : (sword)v.value;
   //    double theValue = value / factor;
   //    tableValueFacts->setValue("VALUE", theValue);
   //    tableValueFacts->update();
   //    count++;
   // }

   // selectAllValueFacts->freeResult();
   // tell(eloAlways, "Updated %d example values", count);

   // ---------------------------------
   // add default for digital outputs

   added = 0;
   count = 0;
   modified = 0;

   for (int f = selectAllMenuItems->find(); f; f = selectAllMenuItems->fetch())
   {
      char* name {nullptr};
      const char* type {nullptr};
      int structType = tableMenu->getIntValue("TYPE");
      std::string sname = tableMenu->getStrValue("TITLE");

      switch (structType)
      {
         case mstDigOut: type = "DO"; break;
         case mstDigIn:  type = "DI"; break;
         case mstAnlOut: type = "AO"; break;
      }

      if (!type)
         continue;

      removeCharsExcept(sname, nameChars);
      asprintf(&name, "%s_0x%x", sname.c_str(), (int)tableMenu->getIntValue("ADDRESS"));

      const char* unit = tableMenu->getStrValue("UNIT");

      if (isEmpty(unit) && strcmp(type, "AO") == 0)
         unit = "%";

      int res = addValueFact(tableMenu->getIntValue("ADDRESS"), type, 1, name, unit,
                             tableMenu->getStrValue("TITLE"));

      if (res == 1)
         added++;
      else if (res == 2)
         modified++;

      free(name);
      count++;
   }

   selectAllMenuItems->freeResult();
   tell(eloAlways, "Checked %d digital lines, added %d, modified %d", count, added, modified);

   // ---------------------------------
   // add value definitions for special data

   addValueFact(udState, "UD", 1, "Status", "zst", "Heizungsstatus");
   tableValueFacts->clear();
   tableValueFacts->setValue("ADDRESS", udState);      // 1  -> Kessel Status
   tableValueFacts->setValue("TYPE", "UD");            // UD -> User Defined
   tableValueFacts->find();
   tableValueFacts->setValue("STATE", "A");
   tableValueFacts->store();

   addValueFact(udMode, "UD", 1, "Betriebsmodus", "txt", "Betriebsmodus");
   tableValueFacts->clear();
   tableValueFacts->setValue("ADDRESS", udMode);       // 2  -> Kessel Mode
   tableValueFacts->setValue("TYPE", "UD");            // UD -> User Defined
   tableValueFacts->find();
   tableValueFacts->setValue("STATE", "A");
   tableValueFacts->store();

   addValueFact(udTime, "UD", 1, "Uhrzeit", "txt", "Datum Uhrzeit der Heizung");
   tableValueFacts->clear();
   tableValueFacts->setValue("ADDRESS", udTime);       // 3  -> Kessel Zeit
   tableValueFacts->setValue("TYPE", "UD");            // UD -> User Defined
   tableValueFacts->find();
   tableValueFacts->setValue("STATE", "A");
   tableValueFacts->store();

   return success;
}

//***************************************************************************
// Dispatch Mqtt Command Request
//   Format:  '{ "command" : "parstore", "address" : 0, "value" : "9" }'
//***************************************************************************

int P4d::dispatchMqttHaCommandRequest(json_t* jData, const char* topic)
{
   const char* command = getStringFromJson(jData, "command", "");

   if (isEmpty(command))
      return Daemon::dispatchMqttHaCommandRequest(jData, topic);

   if (strcmp(command, "parget") == 0)
   {
      int status {fail};
      json_t* jAddress = getObjectFromJson(jData, "address");
      int address = getIntFromJson(jData, "address", -1);

      if (!json_is_integer(jAddress) || address == -1)
      {
         tell(eloAlways, "Error: Missing address or invalid object type for MQTT command 'parget', ignoring");
         return fail;
      }

      tell(eloAlways, "Perform MQTT command '%s' for address %d", command, address);

      ConfigParameter p(address);

      sem->p();
      status = request->getParameter(&p);
      sem->v();

      if (status != success)
      {
         tell(eloAlways, "Parameter request failed!");
         return fail;
      }

      tell(eloAlways, "Address: 0x%4.4x; Unit: %s; Value: %.*f", p.address, p.unit, p.digits, p.rValue);
   }
   else if (strcmp(command, "parstore") == 0)
   {
      int status {fail};
      json_t* jAddress = getObjectFromJson(jData, "address");
      int address = getIntFromJson(jData, "address", -1);
      const char* value = getStringFromJson(jData, "value");

      if (!json_is_integer(jAddress) || address == -1)
      {
         tell(eloAlways, "Error: Missing address or invalid object type for MQTT command 'parstore', ignoring");
         return fail;
      }

      if (isEmpty(value))
      {
         tell(eloAlways, "Error: Missing value for MQTT command 'parstore', ignoring");
         return fail;
      }

      tell(eloAlways, "Perform MQTT command '%s' for address %d with value '%s'", command, address, value);

      ConfigParameter p(address);

      sem->p();
      status = request->getParameter(&p);
      sem->v();

      if (status != success)
      {
         tell(eloAlways, "Set of parameter failed, query of current setting failed!");
         return fail;
      }

      if (p.setValueDirect(value, p.digits, p.getFactor()) != success)
      {
         tell(eloAlways, "Set of parameter failed, wrong format");
         return fail;
      }

      tell(eloAlways, "Storing value '%s' for parameter at address 0x%x", value, address);
      sem->p();
      status = request->setParameter(&p);
      sem->v();

      if (status == success)
      {
         tell(eloAlways, "Stored parameter");
      }
      else
      {
         tell(eloAlways, "Set of parameter failed, error was (%d)", status);

         if (status == P4Request::wrnNonUpdate)
            tell(eloAlways, "Value identical, ignoring request");
         else if (status == P4Request::wrnOutOfRange)
            tell(eloAlways, "Value out of range");
         else
            tell(eloAlways, "Serial communication error");
      }
   }
   else
   {
      tell(eloAlways, "Error: Got unexpected command '%s' in MQTT request, ignoring", command);
   }

   return success;
}

//***************************************************************************
// Dispatch Node-Red Command Request
//   Format:  '{ "command" : "parstore", "address" : 0, "value" : "9" }'
//***************************************************************************

int P4d::dispatchNodeRedCommand(json_t* jObject)
{
   const char* command = getStringFromJson(jObject, "command", "set");

   if (strcmp(command, "parstore") == 0)
      return dispatchMqttHaCommandRequest(jObject, "");

   return Daemon::dispatchNodeRedCommand(jObject);
}

//***************************************************************************
// s3200 State to Json
//***************************************************************************

int P4d::s3200State2Json(json_t* obj)
{
   json_object_set_new(obj, "time", json_integer(currentState.time));
   json_object_set_new(obj, "state", json_integer(currentState.state));
   json_object_set_new(obj, "stateinfo", json_string(currentState.stateinfo));
   json_object_set_new(obj, "mode", json_integer(currentState.mode));
   json_object_set_new(obj, "modeinfo", json_string(currentState.modeinfo));
   json_object_set_new(obj, "version", json_string(currentState.version));
   json_object_set_new(obj, "image", json_string(getTextImage("UD:0x01", currentState.stateinfo)));

   return done;
}

//***************************************************************************
// Config Choice to Json
//***************************************************************************

int P4d::configChoice2json(json_t* obj, const char* name)
{
   Daemon::configChoice2json(obj, name);

   if (strcmp(name, "heatingType") == 0)
   {
      FileList options;
      int count {0};
      char* path {nullptr};

      asprintf(&path, "%s/img/type", httpPath);

      if (getFileList(path, DT_REG, "png", false, &options, count) == success)
      {
         json_t* oArray = json_array();

         for (const auto& opt : options)
         {
            if (strncmp(opt.name.c_str(), "heating-", strlen("heating-")) != 0)
               continue;

            char* p = strdup(strchr(opt.name.c_str(), '-'));
            *(strrchr(p, '.')) = '\0';
            json_array_append_new(oArray, json_string(p+1));
            free(p);
         }

         json_object_set_new(obj, "options", oArray);
      }

      free(path);
   }
   else if (strcmp(name, "stateMailStates") == 0)
   {
      json_t* oArray = json_array();

      for (int i = 0; stateInfos[i].code != na; i++)
      {
         json_t* oOpt = json_object();
         json_object_set_new(oOpt, "value", json_string(std::to_string(stateInfos[i].code).c_str()));
         json_object_set_new(oOpt, "label", json_string(stateInfos[i].title));
         json_array_append_new(oArray, oOpt);
      }

      json_object_set_new(obj, "options", oArray);
   }

   return done;
}

//***************************************************************************
// Get State Image
//***************************************************************************

const char* P4d::getTextImage(const char* key, const char* text)
{
   static char result[100] = "";
   const char* image {nullptr};

   // it's only for UD / udState

   if (strcmp(key, "UD:0x01") != 0)
      return "";

   int state = FroelingService::toState(text);

   if (state == 0)
      image = "state-error.gif";
   else if (state == 1)
      image = "state-fireoff.gif";
   else if (state == 2)
      image = "state-heatup.gif";
   else if (state == 3)
      image = "state-fire.gif";
   else if (state == 4)
      image = "/state/state-firehold.gif";
   else if (state == 5)
      image = "state-fireoff.gif";
   else if (state == 6)
      image = "state-dooropen.gif";
   else if (state == 7)
      image = "state-preparation.gif";
   else if (state == 8)
      image = "state-warmup.gif";
   else if (state == 9)
      image = "state-heatup.gif";
   else if (state == 15 || state == 70 || state == 69)
      image = "state-clean.gif";
   else if ((state >= 10 && state <= 14) || state == 35 || state == 16)
      image = "state-wait.gif";
   else if (state == 60 || state == 61 || state == 72)
      image = "state-shfire.png";

   if (image)
      sprintf(result, "img/state/%s/%s", iconSet, image);
   else
      sprintf(result, "img/type/heating-%s.png", heatingType);

   return result;
}
