package edu.umsl.pjm8cd.alarm.database;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

import edu.umsl.pjm8cd.alarm.alarm.Alarm;
import edu.umsl.pjm8cd.alarm.database.AlarmDBSchema.TimerTable;

/**
 * Created by Pat on 5/1/2016.
 *
 * Wrapper allowing singleton database access.
 */
public class DBWrapper {
    private static DBWrapper wrapper;

    private SQLiteDatabase database;

    public static DBWrapper get(Context context) {
        if (wrapper == null) {
            wrapper = new DBWrapper(context);
        }
        return wrapper;
    }

    public DBWrapper(Context context) {
        database = new AlarmBaseHelper(context).getWritableDatabase();
    }

    // Updates the timer if it is already in the database or adds it if it is not.
    public void updateOrAddAlarm(Alarm alarm) {
        if(getAlarmFromUUID(alarm.getId()) == null) { // New alarm
            database.insert(TimerTable.NAME, null, getContentValues(alarm));
        } else { // Update Alarm
            ContentValues values = getContentValues(alarm);
            database.update(TimerTable.NAME, values, TimerTable.Cols.UUID + " = '" + alarm.getId().toString() + "'", null);
        }
    }

    public void delete(Alarm alarm) {
        database.delete(TimerTable.NAME, TimerTable.Cols.UUID + " = '" + alarm.getId().toString() + "'", null);
    }

    private ContentValues getContentValues(Alarm alarm) {
        ContentValues values = new ContentValues();
        values.put(TimerTable.Cols.UUID, alarm.getId().toString());
        values.put(TimerTable.Cols.NAME, alarm.getName());
        values.put(TimerTable.Cols.HOURS, alarm.getHours());
        values.put(TimerTable.Cols.MIN, alarm.getMinutes());
        if(alarm.isRunning()) {
            values.put(TimerTable.Cols.RUNNING, 1);
        } else {
            values.put(TimerTable.Cols.RUNNING, 0);
        }

        return values;
    }

    private AlarmCursorWrapper queryContacts(String whereClause, String[] whereArgs) {
        Cursor cursor = database.query(
                TimerTable.NAME,
                null, // Columns - null selects all columns
                whereClause,
                whereArgs,
                null, // groupBy
                null, // having
                null  // orderBy
        );

        return new AlarmCursorWrapper(cursor);
    }

    public List<Alarm> getAllTimers() {
        List<Alarm> alarms = new ArrayList<>();

        AlarmCursorWrapper cursor = queryContacts(null, null);

        cursor.moveToFirst();
        while (!cursor.isAfterLast()) {
            alarms.add(cursor.getContact());
            cursor.moveToNext();
        }
        cursor.close();

        return alarms;
    }

    /* Returns the contact in the database associated with the given UUID or null if the contact doesn't exist */
    public Alarm getAlarmFromUUID(UUID id) {
        AlarmCursorWrapper wrap = queryContacts(TimerTable.Cols.UUID + "='" + id.toString() + "'", null);
        wrap.moveToFirst();
        return wrap.getCount() == 0 ? null : wrap.getContact();
    }
}
