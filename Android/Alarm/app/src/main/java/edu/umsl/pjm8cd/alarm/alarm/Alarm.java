package edu.umsl.pjm8cd.alarm.alarm;

import android.text.format.DateFormat;
import android.util.Log;

import java.util.Calendar;
import java.util.Date;
import java.util.UUID;

import edu.umsl.pjm8cd.alarm.AlarmActivityControler;

/**
 * Created by Pat on 5/1/2016.
 *
 * Alarm model.
 */
public class Alarm {
    private UUID id;
    private String name;
    private int hours;
    private int minutes;
    private boolean running;

    public Alarm(){
        this(UUID.randomUUID());
    }

    public Alarm(UUID id) {
        this.id = id;
    }

    public UUID getId() {
        return id;
    }

    public String getName() {
        return name;
    }

    public void setName(String timerName) {
        this.name = timerName;
    }

    public int getHours() {
        return hours;
    }

    public int getMinutes() {
        return minutes;
    }

    public void setHours(int hours) {
        this.hours = hours;
    }

    public void setMinutes(int minutes) {
        this.minutes = minutes;
    }

    public String getTime() {return hours + ":" + minutes; }

    public boolean isRunning() {
        return running;
    }

    public void setRunning(boolean running) {
        this.running = running;
    }

    // Gives the date when the alarm should fire
    public Date getTimeAsDate() {
        return AlarmActivityControler.getTimeAsDate(hours, minutes);
    }

    // Gives the phone specific time string for when the alarm should fire.
    public String getTimeString(boolean twentyFourHour) {
        return AlarmActivityControler.formatTimeString(hours, minutes, twentyFourHour);
    }
}
