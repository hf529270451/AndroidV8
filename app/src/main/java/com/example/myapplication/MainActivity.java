package com.example.myapplication;

import androidx.appcompat.app.AppCompatActivity;

import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.google.gson.Gson;

import org.w3c.dom.Text;

public class MainActivity extends AppCompatActivity {

    static {
        System.loadLibrary("v8runtime");
    }

    private long mV8ContextPtr;

    private LinearLayout container;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        findViewById(R.id.bt_execute_javascript).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.e("", "");
                evaluateJavaScript("callTestFunction(JSON.stringify({\n" +
                        "    \"tag\":\"text\",\n" +
                        "    \"content\":\"我是<Text>aaaaaaa</Text>\",\n" +
                        "    \"width\":800,\n" +
                        "    \"height\":300,\n" +
                        "    \"backgroundColor\":\"#ff00ff\"\n" +
                        "}))");
            }
        });

        container = findViewById(R.id.ll_parent);
    }

    public native void evaluateJavaScript(String script);

    public void drawNode(final String param) {
        Log.e("V8Core", "drawNode:" + param);
        new Handler(Looper.getMainLooper()).post(new Runnable() {
            @Override
            public void run() {
                Gson gson = new Gson();
                Node node = gson.fromJson(param, Node.class);
                if (node.getTag().equals("text")) {
                    TextView tv = new TextView(MainActivity.this);
                    tv.setText(node.getContent());
                    tv.setBackgroundColor(Color.parseColor(node.getBackgroundColor()));
                    tv.setLayoutParams(new LinearLayout.LayoutParams(node.getWidth(), node.getHeight()));
                    container.addView(tv);
                }

            }
        });
    }
}