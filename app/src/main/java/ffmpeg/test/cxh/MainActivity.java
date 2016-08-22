package ffmpeg.test.cxh;

import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.MediaController;
import android.widget.VideoView;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        final VideoView vvShow = (VideoView)this.findViewById(R.id.videoView);
        Button btnPlayOld = (Button) this.findViewById(R.id.btnPlayOld);
        Button btnPlayTra = (Button) this.findViewById(R.id.btnPlayTra);
        Button btnPlayNew = (Button) this.findViewById(R.id.btnPlayNew);

        MediaController controller = new MediaController(this);
        vvShow.setMediaController(controller);

        btnPlayOld.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                File file=new File(Environment.getExternalStorageDirectory().getPath()+ "/old_in.mp4");
                if(file.exists()){
                    vvShow.setVideoPath(file.getAbsolutePath());
                    vvShow.requestFocus();
                    vvShow.start();
                }
            }
        });

        btnPlayTra.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                DumpFrameTask task = new DumpFrameTask(MainActivity.this);
                task.execute();
            }
        });

        btnPlayNew.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(Intent.ACTION_VIEW);
                intent.setDataAndType(Uri.parse(Environment.getExternalStorageDirectory() + "/new_out.mp4"), "video/mp4");
                startActivity(intent);

//                if(file.exists()){
//                    vvShow.setVideoPath(file.getAbsolutePath());
//                    vvShow.requestFocus();
//                    vvShow.start();
//                }
            }
        });

        copyMp4FromAssets(this);
    }

    public static void copyMp4FromAssets(Context context) {

        File file=new File("/storage/emulated/0/old_in.mp4");

        try {
            if (file.exists()) {
                return;
            }
            InputStream is = context.getAssets().open("input.mp4");
            file.createNewFile();
            FileOutputStream fos = new FileOutputStream(file);
            byte[] temp = new byte[1024];
            int i = 0;
            while ((i = is.read(temp)) > 0) {
                fos.write(temp, 0, i);
            }
            fos.close();
            is.close();
        } catch (IOException e) {
            e.printStackTrace();
            return;
        }
    }

    private static class DumpFrameTask extends AsyncTask<Void, Integer, Void> {
        ProgressDialog mlDialog;
        MainActivity mlOuterAct;
        DumpFrameTask(MainActivity pContext) {
            mlOuterAct = pContext;
        }
        @Override
        protected void onPreExecute() {
            mlDialog = ProgressDialog.show(mlOuterAct, "解码中","请等待.." , false);
        }
        @Override
        protected Void doInBackground(Void... params) {
            mp4toyuv(mlOuterAct);
            drawyuv(mlOuterAct);
            yuvtomp4(mlOuterAct);
            File file=new File("/storage/emulated/0/old_in.yuv");
            if(file.exists()){
                file.delete();
            }
            file=new File("/storage/emulated/0/new_out.yuv");
            if(file.exists()){
                file.delete();
            }
            return null;
        }
        @Override
        protected void onPostExecute(Void param) {
            if (null != mlDialog && mlDialog.isShowing()) {
                mlDialog.dismiss();
            }
        }
    }

    private static native int mp4toyuv(MainActivity pObject);
    private static native int drawyuv(MainActivity pObject);
    private static native int yuvtomp4(MainActivity pObject);

    static {
        System.loadLibrary("swresample-2");
        System.loadLibrary("avutil-55");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avformat-57");
        System.loadLibrary("swscale-4");
        System.loadLibrary("ffmpegtest");
    }
}
