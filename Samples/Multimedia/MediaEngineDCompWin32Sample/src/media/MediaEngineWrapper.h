// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "MediaFoundationHelpers.h"
#include "MediaEngineExtension.h"

namespace media
{

// This class handles creation and management of the MediaFoundation
// MediaEngine.
// - It uses the provided IMFMediaSource to feed media samples into the
//   MediaEngine pipeline.
// - A DirectComposition surface is exposed to the application layer to
//   incorporate video from the MediaEngine in its visual tree.
// - The application must provide a callback object (std::function) which
//   is invoked after the media engine has loaded and the DirectComposition surface is available.
class MediaEngineWrapper : public winrt::implements<MediaEngineWrapper, IUnknown>
{
  public:
    using ErrorCB = std::function<void(MF_MEDIA_ENGINE_ERR,HRESULT)>;

    enum class BufferingState
    {
        HAVE_NOTHING = 0,
        HAVE_ENOUGH = 1
    };
    using BufferingStateChangeCB = std::function<void(BufferingState)>;

    MediaEngineWrapper(std::function<void()> initializedCB, ErrorCB errorCB, BufferingStateChangeCB bufferingStateChangeCB,
                       std::function<void()> playbackEndedCB, std::function<void()> timeUpdateCB)
        : m_initializedCB(initializedCB), m_errorCB(errorCB), m_bufferingStateChangeCB(bufferingStateChangeCB),
          m_playbackEndedCB(playbackEndedCB), m_timeUpdateCB(timeUpdateCB)
    {
        // The initialize callback is required
        THROW_HR_IF(E_INVALIDARG, !m_initializedCB);
    }
    ~MediaEngineWrapper() {}

    // Create the media engine with the provided media source
    void Initialize(IMFMediaSource* mediaSource);

    // Stop playback and cleanup resources
    void Pause();
    void Shutdown();

    // Control various aspects of playback
    void StartPlayingFrom(uint64_t timeStamp);
    void SetPlaybackRate(double playbackRate);
    void SetVolume(float volume);

    // Query the current playback position
    uint64_t GetMediaTime();

    // Get a handle to a DCOMP surface for integrating into a visual tree
    HANDLE GetSurfaceHandle();

    // Inform media engine of output window position & size changes
    void OnWindowUpdate(uint32_t width, uint32_t height);

  private:
    wil::critical_section m_lock;
    std::function<void()> m_initializedCB;
    ErrorCB m_errorCB;
    BufferingStateChangeCB m_bufferingStateChangeCB;
    std::function<void()> m_playbackEndedCB;
    std::function<void()> m_timeUpdateCB;
    MFPlatformRef m_platformRef;
    winrt::com_ptr<IMFMediaEngine> m_mediaEngine;
    UINT m_deviceResetToken = 0;
    winrt::com_ptr<IMFDXGIDeviceManager> m_dxgiDeviceManager;
    winrt::com_ptr<MediaEngineExtension> m_mediaEngineExtension;
    winrt::com_ptr<IMFMediaEngineNotify> m_callbackHelper;
    wil::unique_handle m_dcompSurfaceHandle;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    bool m_hasSetSource = false;
    void InitializeVideo();
    void CreateMediaEngine(IMFMediaSource* mediaSource);
    void OnLoaded();
    void OnError(MF_MEDIA_ENGINE_ERR error, HRESULT hr);
    void OnBufferingStateChange(BufferingState state);
    void OnPlaybackEnded();
    void OnTimeUpdate();
};

} // namespace media