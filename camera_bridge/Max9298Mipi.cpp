/*
 * Copyright (C) 2012-2016 Freescale Semiconductor, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Max9298Mipi.h"

Max9298Mipi::Max9298Mipi(int32_t id, int32_t facing, int32_t orientation, char* path)
    : Camera(id, facing, orientation, path)
{
    mVideoStream = new OvStream(this);
}

Max9298Mipi::~Max9298Mipi()
{
}

int Max9298Mipi::OvStream::getCaptureMode(int width, int height)
{
    int index = 0;
    int ret = 0;
    int capturemode = 0;
    struct v4l2_frmsizeenum vid_frmsize;

    while (ret == 0) {
        vid_frmsize.index = index++;
        vid_frmsize.pixel_format = v4l2_fourcc('Y', 'U', 'Y', 'V');
        ret = ioctl(mDev, VIDIOC_ENUM_FRAMESIZES, &vid_frmsize);
        if ((vid_frmsize.discrete.width == (uint32_t)width) && (vid_frmsize.discrete.height == (uint32_t)height)
            && (ret == 0)) {
            capturemode = vid_frmsize.index;
            break;
        }
    }

    return capturemode;
}

status_t Max9298Mipi::initSensorStaticData()
{
    int32_t fd = open(mDevPath, O_RDWR);
    if (fd < 0) {
        ALOGE("Max9298CameraDevice: initParameters sensor has not been opened");
        return BAD_VALUE;
    }

    // first read sensor format.
    int ret = 0, index = 0;
    int sensorFormats[MAX_SENSOR_FORMAT];

    memset(sensorFormats, 0, sizeof(sensorFormats));

    // Don't support enum format, now hard code here.
    sensorFormats[index++] = v4l2_fourcc('Y', 'U', 'Y', 'V');
    mSensorFormatCount =
        changeSensorFormats(sensorFormats, mSensorFormats, index);
    if (mSensorFormatCount == 0) {
        ALOGE("%s no sensor format enum", __func__);
        close(fd);
        return BAD_VALUE;
    }

    mAvailableFormatCount = mSensorFormatCount;

    index = 0;
    char TmpStr[20];
    uint32_t fps = 0;
    int previewCnt = 0, pictureCnt = 0;
    struct v4l2_frmsizeenum vid_frmsize;
    struct v4l2_frmivalenum vid_frmval;
    while (ret == 0) {
        memset(TmpStr, 0, 20);
        memset(&vid_frmsize, 0, sizeof(struct v4l2_frmsizeenum));
        vid_frmsize.index = index++;
        vid_frmsize.pixel_format =
            convertPixelFormatToV4L2Format(mSensorFormats[0]);
        ret = ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &vid_frmsize);
        if (ret != 0) {
            continue;
        }
        ALOGI("enum frame size w:%d, h:%d", vid_frmsize.discrete.width, vid_frmsize.discrete.height);

        if (vid_frmsize.discrete.width == 0 ||
              vid_frmsize.discrete.height == 0) {
            continue;
        }

        vid_frmval.index = 0;
        vid_frmval.pixel_format = vid_frmsize.pixel_format;
        vid_frmval.width = vid_frmsize.discrete.width;
        vid_frmval.height = vid_frmsize.discrete.height;
        while (ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &vid_frmval) >= 0) {
            fps = vid_frmval.discrete.denominator / vid_frmval.discrete.numerator;
            vid_frmval.index++;
        }

        // If w/h ratio is not same with senserW/sensorH, framework assume that
        // first crop little width or little height, then scale.
        // But 1920x1080, 176x144 not work in this mode.
        // For 1M pixel, 720p sometimes may take green picture(5%), so not report
        // it,
        // use 1024x768 for 1M pixel
        // 1920x1080 1280x720 is required by CTS.
        mPictureResolutions[pictureCnt++] = vid_frmsize.discrete.width;
        mPictureResolutions[pictureCnt++] = vid_frmsize.discrete.height;
        mPreviewResolutions[previewCnt++] = vid_frmsize.discrete.width;
        mPreviewResolutions[previewCnt++] = vid_frmsize.discrete.height;
    }  // end while

    if(fps == 0) {
        ALOGE("%s fps info enum fail", __func__);
        close(fd);
        return BAD_VALUE;
    }

    mPreviewResolutionCount = previewCnt;
    mPictureResolutionCount = pictureCnt;

    mMinFrameDuration = 1000001760L / fps;//40001760L;
    mMaxFrameDuration = 30000000000L;
    int i;
    for (i = 0; i < MAX_RESOLUTION_SIZE && i < pictureCnt; i += 2) {
        ALOGI("SupportedPictureSizes: %d x %d", mPictureResolutions[i], mPictureResolutions[i + 1]);
    }

    adjustPreviewResolutions();
    for (i = 0; i < MAX_RESOLUTION_SIZE && i < previewCnt; i += 2) {
        ALOGI("SupportedPreviewSizes: %d x %d", mPreviewResolutions[i], mPreviewResolutions[i + 1]);
    }
    ALOGI("FrameDuration is %" PRId64 ", %" PRId64 "", mMinFrameDuration, mMaxFrameDuration);

    i = 0;
    mTargetFpsRange[i++] = fps;
    mTargetFpsRange[i++] = fps;
    mTargetFpsRange[i++] = fps;
    mTargetFpsRange[i++] = fps;
    mTargetFpsRange[i++] = fps;
    mTargetFpsRange[i++] = fps;

    setMaxPictureResolutions();
    ALOGI("mMaxWidth:%d, mMaxHeight:%d", mMaxWidth, mMaxHeight);

    mFocalLength = 3.37f;
    mPhysicalWidth = 1.4f * mPreviewResolutions[0] / 1000;
    mPhysicalHeight = 1.4f * mPreviewResolutions[1] / 1000;
    mActiveArrayWidth = mPreviewResolutions[0];
    mActiveArrayHeight = mPreviewResolutions[1];
    mPixelArrayWidth = mPreviewResolutions[0];
    mPixelArrayHeight = mPreviewResolutions[1];

    ALOGI("ImxdpuCsi, mFocalLength:%f, mPhysicalWidth:%f, mPhysicalHeight %f",
          mFocalLength,
          mPhysicalWidth,
          mPhysicalHeight);

    close(fd);
    return NO_ERROR;
}

PixelFormat Max9298Mipi::getPreviewPixelFormat()
{
    ALOGI("%s", __func__);
    return HAL_PIXEL_FORMAT_YCbCr_422_I;
}

// configure device.
int32_t Max9298Mipi::OvStream::onDeviceConfigureLocked()
{
    ALOGI("%s", __func__);
    int32_t ret = 0;
    if (mDev <= 0) {
        ALOGE("%s invalid fd handle", __func__);
        return BAD_VALUE;
    }

    uint32_t fps = 0;
    int32_t vformat;
    vformat = convertPixelFormatToV4L2Format(mFormat);

    struct v4l2_frmivalenum frmival;
    frmival.index = 0;
    frmival.pixel_format = vformat;
    frmival.width = mWidth;
    frmival.height = mHeight;

    if(ioctl(mDev, VIDIOC_ENUM_FRAMEINTERVALS, &frmival) == 0) {
            fps = frmival.discrete.denominator / frmival.discrete.numerator;
    }
    else {
        ALOGE("%s fps info enum fail", __func__);
        return BAD_VALUE;
    }

    mFps = fps;

    ALOGI("Width * Height %d x %d format %c%c%c%c, fps: %d", mWidth, mHeight, vformat & 0xFF, (vformat >> 8) & 0xFF,
        (vformat >> 16) & 0xFF, (vformat >> 24) & 0xFF, fps);

    struct v4l2_streamparm param;
    memset(&param, 0, sizeof(param));
    param.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    param.parm.capture.timeperframe.numerator   = 1;
    param.parm.capture.timeperframe.denominator = fps;
    param.parm.capture.capturemode = getCaptureMode(mWidth, mHeight);
    ret = ioctl(mDev, VIDIOC_S_PARM, &param);
    if (ret < 0) {
        ALOGE("%s: VIDIOC_S_PARM Failed: %s", __func__, strerror(errno));
        return ret;
    }

    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    fmt.fmt.pix_mp.pixelformat = vformat;
    fmt.fmt.pix_mp.width = mWidth & 0xFFFFFFF8;
    fmt.fmt.pix_mp.height = mHeight & 0xFFFFFFF8;
    fmt.fmt.pix_mp.num_planes = 1; /* max9298 camera use YUYV format, is packed storage mode, set num_planes 1*/

    ret = ioctl(mDev, VIDIOC_S_FMT, &fmt);
    if (ret < 0) {
        ALOGE("%s: VIDIOC_S_FMT Failed: %s", __func__, strerror(errno));
        return ret;
    }

    return 0;
}
