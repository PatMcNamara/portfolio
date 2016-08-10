package edu.umsl.pjm8cd.alarm.database;

import android.database.Cursor;
import android.database.CursorWrapper;

import java.util.UUID;

import edu.umsl.pjm8cd.alarm.alarm.Alarm;
import edu.umsl.pjm8cd.alarm.database.AlarmDBSchema.TimerTable;

/**
 * Created by Pat on 5/1/2016.
 */
public class AlarmCursorWrapper extends CursorWrapper {
    public AlarmCursorWrapper(Cursor cursor) {
        super(cursor);
    }

    public Alarm getContact() {
        String uuidString = getString(getColumnIndex(TimerTable.Cols.UUID));
        String name = getString(getColumnIndex(TimerTable.Cols.NAME));
        int min = getInt(getColumnIndex(TimerTable.Cols.MIN));
        int hours = getInt(getColumnIndex(TimerTable.Cols.HOURS));
        int running = getInt(getColumnIndex(TimerTable.Cols.RUNNING));

        Alarm c = new Alarm(UUID.fromString(uuidString));
        c.setName(name);
        c.setHours(hours);
        c.setMinutes(min);
        c.setRunning(running != 0);
        return c;
    }
}
