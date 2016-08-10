package edu.umsl.pjm8cd.alarm.database;

/**
 * Created by Pat on 5/1/2016.
 */
public class AlarmDBSchema {
    public static final class TimerTable {
        public static final String NAME = "timer";

        public static final class Cols {
            public static final String UUID = "uuid";
            public static final String NAME = "name";
            public static final String HOURS = "hours";
            public static final String MIN = "min";
            public static final String RUNNING = "running";
        }
    }
}
