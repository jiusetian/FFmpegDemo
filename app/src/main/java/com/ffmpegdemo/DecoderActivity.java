package com.ffmpegdemo;

import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

public class DecoderActivity extends AppCompatActivity {

    Handler handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            //显示某个吐司
            if (msg.what == 1) {
                Log.d("tag", "handleMessage: 执行吐司");
                Toast.makeText(DecoderActivity.this, (String)msg.obj, Toast.LENGTH_SHORT);
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_decoder);

        Button startButton = (Button) this.findViewById(R.id.bt_start);
        final EditText urlEdittext_input = (EditText) this.findViewById(R.id.et01);
        final EditText urlEdittext_output = (EditText) this.findViewById(R.id.editText2);

        startButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View arg0) {

                String folderurl = Environment.getExternalStorageDirectory().getPath() + "/DCIM/Camera";

                String urltext_input = urlEdittext_input.getText().toString();
                final String inputurl = folderurl + "/" + urltext_input;

                String urltext_output = urlEdittext_output.getText().toString();
                final String outputurl = folderurl + "/" + urltext_output;
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        final int i = AvUtil.decoder(inputurl, outputurl);
                        Log.d("tag", "run:i的值为"+i);
                        Message message = Message.obtain();
                        message.what = 1;
                        message.obj = i == 0 ? "解码成功" : "解码失败";
                        handler.sendMessage(message);
                    }
                }).start();

            }
        });

    }
}
















