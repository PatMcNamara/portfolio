package edu.umsl.pjm8cd.alarm.alarm;

import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.text.format.DateFormat;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import java.util.List;

import edu.umsl.pjm8cd.alarm.R;
import edu.umsl.pjm8cd.alarm.database.DBWrapper;

/**
 * Created by Pat on 4/17/2016.
 *
 * Displays a list of alarms. (One half of the tabbed main view)
 */
public class ListViewFragment extends Fragment {
    private RecyclerView contactRecyclerView;
    private List<Alarm> alarms;

    // This text view is shown if the list is empty.
    private TextView noAlarms;

    public static ListViewFragment newInstance() {
        ListViewFragment fragment = new ListViewFragment();
        return fragment;
    }

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.alarm_list_layout, container, false);

        noAlarms = (TextView) view.findViewById(R.id.no_alarms);

        contactRecyclerView = (RecyclerView) view.findViewById(R.id.contacts_recycler_view);
        contactRecyclerView.setLayoutManager(new LinearLayoutManager(getContext()));

        alarms = DBWrapper.get(getContext()).getAllTimers();
        contactRecyclerView.setAdapter(new TimerAdapter());

        setHasOptionsMenu(true);
        return view;
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        super.onCreateOptionsMenu(menu, inflater);
        inflater.inflate(R.menu.alarm, menu);
    }
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        Intent intent = AlarmDetailActivity.newIntent(getContext());
        startActivity(intent);
        return super.onOptionsItemSelected(item);
    }

    @Override
    public void onResume() {
        super.onResume();
        updateView();
    }

    /* Updates the alarms in the recycler view */
    public void updateView() {
        alarms = DBWrapper.get(getContext()).getAllTimers();
        contactRecyclerView.setAdapter(new TimerAdapter());
        if(isEmpty()) {
            noAlarms.setVisibility(View.VISIBLE);
        } else {
            noAlarms.setVisibility(View.GONE);
        }
    }

    /* Are there any alarms? */
    public boolean isEmpty() {
        if(alarms == null)
            return true;
        return alarms.isEmpty();
    }

    private class TimerHolder extends RecyclerView.ViewHolder implements View.OnClickListener {
        private TextView nameView;
        private TextView timeView;
        private View itemView;

        private Alarm alarm;

        public TimerHolder(View itemView) {
            super(itemView);
            itemView.setOnClickListener(this);

            nameView = (TextView) itemView.findViewById(R.id.name_text);
            timeView = (TextView) itemView.findViewById(R.id.time_text);
            this.itemView = itemView;
        }

        public void bindContact(Alarm a) {
            alarm = a;

            nameView.setText(alarm.getName());
            timeView.setText(alarm.getTimeString(DateFormat.is24HourFormat(getActivity())));

            // Running alarms should have different color.
            if(alarm.isRunning()) {
                itemView.setBackgroundColor(getResources().getColor(android.R.color.holo_green_light));
            }
        }

        @Override
        public void onClick(View view) {
            // Open the details view of the alarm
            Intent intent = AlarmDetailActivity.newIntent(getContext(), alarm);
            startActivity(intent);
        }
    }

    private class TimerAdapter extends RecyclerView.Adapter<TimerHolder> {
        @Override
        public TimerHolder onCreateViewHolder(ViewGroup parent, int viewType) {
            LayoutInflater inflater = LayoutInflater.from(getContext());
            View view = inflater.inflate(R.layout.contact_item_layout, parent, false);
            return new TimerHolder(view);
        }

        @Override
        public void onBindViewHolder(TimerHolder holder, int position) {
            Alarm c = alarms.get(position);
            holder.bindContact(c);
        }

        @Override
        public int getItemCount() {
            return alarms.size();
        }
    }
}
