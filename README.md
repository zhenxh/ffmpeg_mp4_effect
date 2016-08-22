主要的参考内容http://blog.csdn.net/leixiaohua1020/article/details/50618190


 在windows环境使用android studio编译ffmpeg，通过3个步骤修改视频源文件。

 1,mp4文件转换成yuv

 2,yuv上做特效处理转成新的yuv文件

 3,新的yuv文件转换回mp4格式

 

 还有部分没有完成

 1，在视频文件上添加文字或图片，通过尝试的几种方案，判断应该是添加资源路径或指令集在android上支持的问题。

 2，yuv转回的mp4文件不完整，没有时间信息，只能通过视频播放器播放看添加的效果。

 

 目前输入文件old_in.mp4 和输出文件 new_out.mp4都保存在sdcard根目录。

 apk使用方法直接点击视频转换。实现特效为在原视频上添加粉色框。
