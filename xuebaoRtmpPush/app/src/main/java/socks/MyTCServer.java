package socks;

import android.os.Message;
import android.util.Log;

import org.json.JSONObject;

import socks.VideoConfig;

import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.InputStreamReader;
import java.net.ServerSocket;
import java.net.Socket;

//监听内网的7777端口以便接收配置变更
public class MyTCServer {
    public static final int PORT = 7777;//监听的端口号

    private Thread newThread; //声明一个子线程

    boolean ShouldStopNow = false;

    public void init() {

        ShouldStopNow = false;
        newThread = new Thread(new Runnable() {
            @Override
            public void run() {
                //这里写入子线程需要做的工作
                try {
                    ServerSocket serverSocket = new ServerSocket(PORT);
                    while (ShouldStopNow == false) {
                        // 一旦有堵塞, 则表示服务器与客户端获得了连接
                        Socket client = serverSocket.accept();
                        // 处理这次连接
                        new HandlerThread(client);
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        newThread.start(); //启动线程
    }

    public void StopNow()
    {
        ShouldStopNow = true;
        if( newThread != null)
        {
            newThread.interrupt(); newThread = null;
        }
    }

    private class HandlerThread implements Runnable {
        private Socket socket;

        public HandlerThread(Socket client) {
            socket = client;
            new Thread(this).start();
        }

        public void run() {
            try {
                DataInputStream input = new DataInputStream(socket.getInputStream());
                DataOutputStream out = new DataOutputStream(socket.getOutputStream());

                while (ShouldStopNow == false) {
                    byte jj[] = new byte[2048];
                    int r_len = input.read(jj);
                    if(r_len<=0)
                        break;

                    String aa = new String(jj,0,r_len, "UTF-8");

                    Log.e("recv json?", aa);
                    JSONObject jsonObject = new JSONObject(aa);
                    if(jsonObject.has("cmd") == false)
                    {
                        Log.e("没有CMD","关闭此连接");
                        break;
                    }

                    String cmd = jsonObject.getString("cmd");
                    if(cmd.equals("getlist"))
                    {
                        JSONObject jsonObjectR = new JSONObject();
                        jsonObjectR.put("mac", VideoConfig.instance.my_mac);
                        jsonObjectR.put("name",  VideoConfig.instance.machine_name);

                        String strR = jsonObjectR.toString();
                        out.write(strR.getBytes(), 0, strR.getBytes().length);
                        out.flush();
                    }else if(cmd.equals("getconfig"))
                    {
                        String s = VideoConfig.instance.makeJson();
                        out.write(s.getBytes(), 0, s.getBytes().length);
                        out.flush();
                    }else if(cmd.equals("applyconfig"))
                    {
                        boolean apply_ret = VideoConfig.instance.ApplyConfig(aa, socket);
                        Log.e("收到配置数据", aa);
                        /*if(apply_ret)
                        {
                            String s = "{\"result\":\"ok\"}";
                            out.write(s.getBytes(), 0, s.getBytes().length);
                            out.flush();
                        } else {
                            String s = "{\"result\":\"failed\"}";
                            out.write(s.getBytes(), 0, s.getBytes().length);
                            out.flush();
                        }*/
                    }else if(cmd.equals("update"))
                    {
                        //todo update命令的时候 执行更新
                       // cmd=="update"  url  versionCode
                        //String url = jsonObject.getString("url");
                       // int versionCode= jsonObject.getInt("versionCode");

                        if( jsonObject.has("url") == false )
                        {
                            String s = "{\"result\":\"failed\"}";
                            out.write(s.getBytes(), 0, s.getBytes().length);
                            out.flush();
                        }
                        else {
                            String s = "{\"result\":\"ok\"}";
                            out.write(s.getBytes(), 0, s.getBytes().length);
                            out.flush();
                        }

                        Message message = Message.obtain();
                        message.what = 110;//软件自动更新
                        message.obj = aa;

                        if( VideoConfig.instance.msgHandler  != null)
                            VideoConfig.instance.msgHandler.sendMessage(message);

                        //{"cmd":"update", "url":"https://ssfasdfasdf", "versionCode":2}
                    }

                }

                if (socket != null) {
                    try {
                        socket.close();
                    } catch (Exception e) {
                        socket = null;
                        System.out.println("数据异常.close.:" + e.getMessage());
                    }
                }

            } catch (Exception e) {
                System.out.println("数据异常。close。: " + e.getMessage());

            } finally {
                if (socket != null) {
                    try {
                        socket.close();
                    } catch (Exception e) {
                        socket = null;
                        System.out.println("服务端 finally 异常:" + e.getMessage());
                    }
                }
            }
        }
    }
}

