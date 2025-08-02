#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2024 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/animation/CameraSampler.h>
#include <vsg/core/Inherit.h>
#include <vsg/maths/quat.h>
#include <vsg/ui/KeyEvent.h>

namespace vsg
{

    /// event handler for controlling the playing and recording of camera animation paths
    class VSG_DECLSPEC CameraAnimationHandler : public Inherit<Visitor, CameraAnimationHandler>
    {
    public:
        CameraAnimationHandler();
        CameraAnimationHandler(ref_ptr<Object> in_object, ref_ptr<Animation> in_animation, const Path& in_filename = "saved_animation.vsgt", ref_ptr<Options> in_options = {});
        explicit CameraAnimationHandler(ref_ptr<Object> in_object, const Path& in_filename = "saved_animation.vsgt", ref_ptr<Options> in_options = {});

        /// object to track/modify
        ref_ptr<Object> object;

        /// file to read/write to
        Path filename;
        ref_ptr<Options> options;

        // animation to play/record to
        ref_ptr<Animation> animation;

        // CameraSampler to play/record to
        ref_ptr<CameraSampler> cameraSampler;

        KeySymbol toggleRecordingKey = KEY_r;
        KeySymbol togglePlaybackKey = KEY_p;

        bool recording = false;
        bool playing = false;
        double simulationTime = 0.0;
        double startTime = 0.0;

        void play();
        void record();
        void stop();

    public:
        void apply(Camera& camera) override;
        void apply(MatrixTransform& transform) override;

        void apply(KeyPressEvent& keyPress) override;
        void apply(FrameEvent& frame) override;

    protected:
    };
    VSG_type_name(vsg::CameraAnimationHandler);

    // fallback for naming prior to VulkanSceneGraph-1.1.9.
    using CameraAnimation = vsg::CameraAnimationHandler;

} // namespace vsg
