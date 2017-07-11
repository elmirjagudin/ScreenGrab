using System;
using System.IO;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;

using System.Drawing;
using System.Drawing.Imaging;
using System.Threading;

public class ScreenGrabber : MonoBehaviour
{
    AutoResetEvent SaveImage;
    bool GrabImage;
    bool StopSavingImages = false;

    Thread ImageSaverThread;

    [DllImport("Plugin")]
    private static extern IntPtr GetRenderEventFunc();

    [DllImport("Plugin")]
    private static extern IntPtr GetMappedResource(ref uint width, ref uint height, ref uint RowPitch);

    int _cntr = 0;
    int cntr
    {
        get
        {
            if (_cntr > 32)
            {
                _cntr = 0;
            }
            return _cntr++;
        }
    }

    IEnumerator Start()
    {
        SaveImage = new AutoResetEvent(false);
        GrabImage = true;
        StopSavingImages = false;

        ImageSaverThread = new Thread(WriteImages);
        ImageSaverThread.Start();

        yield return StartCoroutine("CallPluginAtEndOfFrames");
        
    }

    void OnDisable()
    {
        Debug.Log("screen grabber disabled, stopping save image thread");
        StopSavingImages = true;

        ImageSaverThread.Join();
        Debug.Log("yep, it's done");
    }

    void WriteImages()
    {
        try
        {
            while (!StopSavingImages)
            {
                Debug.Log("waiting for image save signal");
                SaveImage.WaitOne();
                MakePng();
                Debug.Log("grab next please");
                GrabImage = true;
            }
        }
        catch (Exception e)
        {
            Debug.LogException(e);
        }

        Debug.Log("image saver thread finished");
    }

    unsafe void MakePng()
    {
        uint width = 0;
        uint height = 0;
        uint RowPitch = 0;

        IntPtr dataPntr = GetMappedResource(ref width, ref height, ref RowPitch);

        Debug.LogFormat("width {0} height {1}  RowPitch {2}", width, height, RowPitch);

        byte* data = (byte*)dataPntr.ToPointer();

        var bitmap = new Bitmap((int)width, (int)height);
        for (uint y = 0; y < height; y++)
        {
            for (uint x = 0; x < width; x++)
            {
                uint rowStart = RowPitch * y;
                uint pos = rowStart + x * 4;
                //uint pos = (x + y * width) * 4;

                var c = System.Drawing.Color.FromArgb(data[pos], data[pos + 1], data[pos + 2]);

                bitmap.SetPixel((int)x, (int)y, c);
            }
        }

        var name = String.Format(@"C:\Users\brab\Desktop\shots\im{0}.png", cntr);
        bitmap.Save(name, ImageFormat.Png);
        Debug.LogFormat("saved to {0}", name);
    }

    void Encode()
    {
        MakePng();
    }

    IEnumerator CallPluginAtEndOfFrames()
    {
        while (true)
        {
            yield return new WaitForEndOfFrame();
            GL.IssuePluginEvent(GetRenderEventFunc(), 1);         /* copy */

            yield return new WaitForEndOfFrame();
            yield return new WaitForEndOfFrame();
            GL.IssuePluginEvent(GetRenderEventFunc(), 2);         /* map */
            yield return new WaitForEndOfFrame();

            GrabImage = false;
            SaveImage.Set();

            Debug.Log("waiting for image to hit the disk");
            yield return new WaitUntil(() => GrabImage);

            Debug.Log("grabbing next image");
            GL.IssuePluginEvent(GetRenderEventFunc(), 3);         /* unmap */
        }
    }
}
