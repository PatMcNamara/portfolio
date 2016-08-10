package edu.umsl.pjm8cd.alarm.database;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

import edu.umsl.pjm8cd.alarm.database.AlarmDBSchema.TimerTable;

/**
 * Created by Pat on 4/17/2016.
 */
public class AlarmBaseHelper extends SQLiteOpenHelper {
    private static final String TAG = "AlarmBaseHelper";
    private static final int VERSION = 1;
    private static final String DATABASE_NAME = "timer.sqlite";

    public AlarmBaseHelper(Context context) {
        super(context, DATABASE_NAME, null, VERSION);
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        db.execSQL("CREATE TABLE " + TimerTable.NAME + " (" +
                "_id integer primary key autoincrement, " +
                TimerTable.Cols.UUID + " TEXT, " +
                TimerTable.Cols.NAME + " TEXT, " +
                TimerTable.Cols.HOURS + " INTEGER, " +
                TimerTable.Cols.MIN + " INTEGER," +
                TimerTable.Cols.RUNNING + " INTEGER" +
                ")"
        );
    }

    @Override
    public void onUpgrade(SQLiteDatabase sqLiteDatabase, int i, int i1) {

    }
}
