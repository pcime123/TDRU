package com.sscctv.tdru;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import java.util.List;

public class Adapter_spinner extends BaseAdapter{

    private Context context;
    private List<String> data;
    private LayoutInflater inflater;

    Adapter_spinner(Context context, List<String> data) {
        this.context = context;
        this.data = data;
        inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    }

    @Override
    public int getCount() {
        if(data != null)
        return data.size();
        else return 0;
    }

    @Override
    public Object getItem(int position) {
        return data.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        if(convertView == null) {
            convertView = inflater.inflate(R.layout.spinner_setting, parent, false);
        }
        String text = data.get(position);
        ((TextView) convertView.findViewById(R.id.spinnerText)).setText(text);
        return convertView;
    }

    @Override
    public View getDropDownView(int position, View convertView, ViewGroup parent) {
        if(convertView == null) {
            convertView = inflater.inflate(R.layout.spinner_dropdown, parent, false);
        }
        String text = data.get(position);
        ((TextView) convertView.findViewById(R.id.spinnerText)).setText(text);
        return convertView;
    }
}
