package edu.umsl.pjm8cd.alarm.alarm;

import android.app.AlarmManager;
import android.app.Dialog;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.TimePickerDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.media.MediaPlayer;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.DialogFragment;
import android.support.v4.app.Fragment;
import android.support.v4.app.NotificationCompat;
import android.text.format.DateFormat;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.TimePicker;
import android.widget.Toast;

import java.util.Calendar;
import java.util.UUID;

import edu.umsl.pjm8cd.alarm.AlarmActivityControler;
import edu.umsl.pjm8cd.alarm.R;
import edu.umsl.pjm8cd.alarm.database.DBWrapper;

/**
 * Created by Pat on 5/1/2016.
 *
 * This view handles the alarm details that come up when a user selects an alarm from the recycler
 */
public class AlarmDetailsView extends Fragment {
    public static final String UUID_KEY = "uuid";

    private EditText alarmName;
    private TextView alarmTime;
    private Switch active;
    private DBWrapper database;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        setRetainInstance(true);
        View view = inflater.inflate(R.layout.edit_alarm, container, false);

        database = new DBWrapper(getContext());

        alarmName = (EditText) view.findViewById(R.id.alarm_name);
        alarmTime = (TextView) view.findViewById(R.id.alarm_time);
        active = (Switch) view.findViewById(R.id.alarm_enabled);

        String specifiedUUID = getActivity().getIntent().getStringExtra(AlarmDetailActivity.UUID);
        if(specifiedUUID != null) { // Is user viewing an existing contact or creating a new one?
            Alarm c = DBWrapper.get(getActivity()).getAlarmFromUUID(UUID.fromString(specifiedUUID));
            fillFields(c);
        }

        ((Button) view.findViewById(R.id.change_time_button)).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                showTimePickerDialog(view);
            }
        });

        ((Button) view.findViewById(R.id.save_button)).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                // Add contact with the given info into the database.
                if (alarmName.getText().toString().isEmpty() || alarmTime.getText().toString().isEmpty()) {
                    Toast.makeText(getContext(), "Alarm must have a name and a time.", Toast.LENGTH_SHORT).show();
                } else {
                    Alarm a = buildContact();
                    database.updateOrAddAlarm(a);

                    // Create broadcast pending intent containing the UUID of the alarm
                    Intent resultIntent = new Intent(getContext(), AlarmReceiver.class);
                    resultIntent.putExtra(UUID_KEY, a.getId().toString());
                    PendingIntent resultPendingIntent = PendingIntent.getBroadcast(getContext(), a.getId().hashCode(), resultIntent, PendingIntent.FLAG_UPDATE_CURRENT);

                    AlarmManager alarm = (AlarmManager) getActivity().getSystemService(Context.ALARM_SERVICE);
                    if(a.isRunning()) { // schedule if the alarm needs to go off.
                        alarm.set(AlarmManager.RTC_WAKEUP, a.getTimeAsDate().getTime(), resultPendingIntent);//TODO FIX this time
                        Log.d("HI", "Started alarm to trigger at " + a.getTimeAsDate().toString());
                    } else { // Remove from alarm manager if it is already there
                        alarm.cancel(resultPendingIntent);
                    }
                    getActivity().finish();
                }
            }
        });

        ((Button) view.findViewById(R.id.delete_button)).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (alarmName.getText().toString().isEmpty() || alarmTime.getText().toString().isEmpty()) {
                    return;
                }
                Alarm a = buildContact();
                database.delete(a);
                getActivity().finish();
                // We don't worry about doing the alarmmanger entry because the broadcast handler will cover it.
            }
        });

        ((Button) view.findViewById(R.id.cancel_button)).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                getActivity().finish();
            }
        });

        return view;
    }

    public void showTimePickerDialog(View v) {
        DialogFragment newFragment = new TimePickerFragment();
        ((TimePickerFragment) newFragment).setAlarmTime(alarmTime);
        newFragment.show(getActivity().getSupportFragmentManager(), "timePicker");
    }

    /* Fills the text areas in the view with the given contact info. */
    private void fillFields(Alarm c) {
        alarmName.setText(c.getName());
        active.setChecked(c.isRunning());
        alarmTime.setText(c.getTime());
    }

    /* Takes the view fields and builds a Alarm. */
    public Alarm buildContact() {
        String name = alarmName.getText().toString();
        String[] time = alarmTime.getText().toString().split(":");

        // This is cludgy and would be better if we just stored the values when they were updated.
        int hour = Integer.parseInt(time[0]);
        int min = Integer.parseInt(time[1]);

        Alarm newTimer;
        // If we are updating an existing contact, use the same UUID
        String contactUUID = getActivity().getIntent().getStringExtra(AlarmDetailActivity.UUID);
        if(contactUUID == null) {
            newTimer = new Alarm();
        } else {
            newTimer = new Alarm(UUID.fromString(contactUUID));
        }

        newTimer.setName(name);
        newTimer.setHours(hour);
        newTimer.setMinutes(min);
        newTimer.setRunning(active.isChecked());
        return newTimer;
    }

    public static class TimePickerFragment extends DialogFragment
            implements TimePickerDialog.OnTimeSetListener {
        private TextView alarmTime;

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            // Use the current time as the default values for the picker (would be better if we brought in the alarm so we could set it to the same values).
            final Calendar c = Calendar.getInstance();
            int hour = c.get(Calendar.HOUR_OF_DAY);
            int minute = c.get(Calendar.MINUTE);

            // Create a new instance of TimePickerDialog and return it
            return new TimePickerDialog(getActivity(), this, hour, minute,
                    DateFormat.is24HourFormat(getActivity()));
        }

        public void onTimeSet(TimePicker view, int hourOfDay, int minute) {
            alarmTime.setText(hourOfDay + ":" + minute); // This really should be changed to match the behavior of the timer in the recycler but then lots of other stuff have to change too.
        }
        public void setAlarmTime(TextView alarmTime) {
            this.alarmTime = alarmTime;
        }
    }

    public static class AlarmReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            DBWrapper database = DBWrapper.get(context);
            Alarm a = database.getAlarmFromUUID(UUID.fromString(intent.getStringExtra(UUID_KEY)));

            if(a == null) { // The delete method doesn't actually remove it from the alarm manager so this check is required.
                return;
            }

            // Set and fire a notification.
            NotificationCompat.Builder builder = new NotificationCompat.Builder(context);
            builder.setSmallIcon(android.R.drawable.ic_dialog_alert);
            builder.setContentTitle("Alarm");
            builder.setContentText(a.getName() + " at time " + a.getTimeString(DateFormat.is24HourFormat(context)));

            Intent resultIntent = new Intent(context, AlarmActivityControler.class);
            PendingIntent resultPendingIntent = PendingIntent.getActivity(context, a.getId().hashCode(), resultIntent, PendingIntent.FLAG_UPDATE_CURRENT);

            builder.setContentIntent(resultPendingIntent);
            int notificationId = a.getId().hashCode();
            NotificationManager manager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
            manager.notify(notificationId, builder.build());

            // Play sound
            MediaPlayer mp = MediaPlayer.create(context, R.raw.finished);
            mp.start();

            // Set alarm to off.
            a.setRunning(false);
            database.updateOrAddAlarm(a);
        }
    }
}
