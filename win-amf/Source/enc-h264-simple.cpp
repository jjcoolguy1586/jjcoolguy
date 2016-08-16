/*
MIT License

Copyright (c) 2016 Michael Fabian Dirks

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "enc-h264-simple.h"
//////////////////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////////////////

// Plugin
#include "win-amf.h"
#include "amf-vce-capabilities.h"

//////////////////////////////////////////////////////////////////////////
// Defines
//////////////////////////////////////////////////////////////////////////
#define AMF_TEXT_H264(x) (AMF_TEXT("H264Simple." ## x))
#define AMF_TEXT_H264_T(x) obs_module_text(AMF_TEXT_H264(x))

#define AMF_VCE_H264_NAME												AMF_TEXT_H264("Name")
#define AMF_VCE_H264_KEYFRAME_INTERVAL									AMF_TEXT_H264("KeyframeInterval")
#define AMF_VCE_H264_QUALITY_PRESET										AMF_TEXT_H264("QualityPreset")
#define AMF_VCE_H264_QUALITY_PRESET_SPEED								AMF_TEXT_H264("QualityPreset.Speed")
#define AMF_VCE_H264_QUALITY_PRESET_BALANCED							AMF_TEXT_H264("QualityPreset.Balanced")
#define AMF_VCE_H264_QUALITY_PRESET_QUALITY								AMF_TEXT_H264("QualityPreset.Quality")
#define AMF_VCE_H264_TUNING												AMF_TEXT_H264("Tuning")
#define AMF_VCE_H264_TUNING_DEFAULT										AMF_TEXT_H264("Tuning.Default")
#define AMF_VCE_H264_TUNING_TWITCH										AMF_TEXT_H264("Tuning.Twitch")
#define AMF_VCE_H264_TUNING_YOUTUBE										AMF_TEXT_H264("Tuning.YouTube")
#define AMF_VCE_H264_TUNING_RECORDING									AMF_TEXT_H264("Tuning.Recording")
#define AMF_VCE_H264_PROFILE											AMF_TEXT_H264("Profile")
#define AMF_VCE_H264_PROFILE_BASELINE									AMF_TEXT_H264("Profile.Baseline")
#define AMF_VCE_H264_PROFILE_MAIN										AMF_TEXT_H264("Profile.Main")
#define AMF_VCE_H264_PROFILE_HIGH										AMF_TEXT_H264("Profile.High")
#define AMF_VCE_H264_RATECONTROL										AMF_TEXT_H264("RateControl")
#define AMF_VCE_H264_RATECONTROL_CQP									AMF_TEXT_H264("RateControl.CQP")
#define AMF_VCE_H264_RATECONTROL_CBR									AMF_TEXT_H264("RateControl.CBR")
#define AMF_VCE_H264_RATECONTROL_VBR_PEAK_CONSTRAINED					AMF_TEXT_H264("RateControl.VBR.Peak")
#define AMF_VCE_H264_RATECONTROL_VBR_LATENCY_CONSTRAINED				AMF_TEXT_H264("RateControl.VBR.Latency")
#define AMF_VCE_H264_BITRATE_TARGET										AMF_TEXT_H264("Bitrate.Target")
#define AMF_VCE_H264_BITRATE_PEAK										AMF_TEXT_H264("Bitrate.Peak")
#define AMF_VCE_H264_QP_MINIMUM											AMF_TEXT_H264("QP.Minimum")
#define AMF_VCE_H264_QP_MAXIMUM											AMF_TEXT_H264("QP.Maximum")
#define AMF_VCE_H264_QP_IFRAME											AMF_TEXT_H264("QP.IFrame")
#define AMF_VCE_H264_QP_PFRAME											AMF_TEXT_H264("QP.PFrame")
#define AMF_VCE_H264_QP_BFRAME											AMF_TEXT_H264("QP.BFrame")
#define AMF_VCE_H264_USE_CUSTOM_BUFFER_SIZE								AMF_TEXT_H264("UseCustomBufferSize")
#define AMF_VCE_H264_CUSTOM_BUFFER_SIZE									AMF_TEXT_H264("CustomBufferSize")
#define AMF_VCE_H264_FRAME_SKIPPING										AMF_TEXT_H264("FrameSkipping")

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
obs_encoder_info* AMFEncoder::VCE_H264_Simple_Encoder::encoder_info;

void AMFEncoder::VCE_H264_Simple_Encoder::encoder_register() {
	if (!AMFEncoder::VCE_H264_Simple_Encoder::encoder_info) {
		AMFEncoder::VCE_H264_Simple_Encoder::encoder_info = new obs_encoder_info();
		AMFEncoder::VCE_H264_Simple_Encoder::encoder_info->id = "amf_h264_simple_encoder";
		AMFEncoder::VCE_H264_Simple_Encoder::encoder_info->type = obs_encoder_type::OBS_ENCODER_VIDEO;
		AMFEncoder::VCE_H264_Simple_Encoder::encoder_info->codec = "h264";

		// Functions
		AMFEncoder::VCE_H264_Simple_Encoder::encoder_info->get_name = &AMFEncoder::VCE_H264_Simple_Encoder::get_name;
		AMFEncoder::VCE_H264_Simple_Encoder::encoder_info->get_defaults = &AMFEncoder::VCE_H264_Simple_Encoder::get_defaults;
		AMFEncoder::VCE_H264_Simple_Encoder::encoder_info->get_properties = &AMFEncoder::VCE_H264_Simple_Encoder::get_properties;
		AMFEncoder::VCE_H264_Simple_Encoder::encoder_info->create = &AMFEncoder::VCE_H264_Simple_Encoder::create;
		AMFEncoder::VCE_H264_Simple_Encoder::encoder_info->destroy = &AMFEncoder::VCE_H264_Simple_Encoder::destroy;
		AMFEncoder::VCE_H264_Simple_Encoder::encoder_info->encode = &AMFEncoder::VCE_H264_Simple_Encoder::encode;
		AMFEncoder::VCE_H264_Simple_Encoder::encoder_info->update = &AMFEncoder::VCE_H264_Simple_Encoder::update;
		AMFEncoder::VCE_H264_Simple_Encoder::encoder_info->get_video_info = &AMFEncoder::VCE_H264_Simple_Encoder::get_video_info;
		AMFEncoder::VCE_H264_Simple_Encoder::encoder_info->get_extra_data = &AMFEncoder::VCE_H264_Simple_Encoder::get_extra_data;

		obs_register_encoder(AMFEncoder::VCE_H264_Simple_Encoder::encoder_info);
	}
}

const char* AMFEncoder::VCE_H264_Simple_Encoder::get_name(void* type_data) {
	return AMF_TEXT_H264_T("Name");
}

void AMFEncoder::VCE_H264_Simple_Encoder::get_defaults(obs_data_t *settings) {
	// Main Properties
	/// Keyframe Interval
	obs_data_set_default_int(settings, AMF_VCE_H264_KEYFRAME_INTERVAL, 2);
	/// Quality Preset
	obs_data_set_default_int(settings, AMF_VCE_H264_QUALITY_PRESET, VCE_QUALITY_PRESET_BALANCED);
	/// Tuning
	obs_data_set_default_int(settings, AMF_VCE_H264_TUNING, 0);
	/// Profile
	obs_data_set_default_int(settings, AMF_VCE_H264_PROFILE, VCE_PROFILE_MAIN);
	/// Rate Control
	obs_data_set_default_int(settings, AMF_VCE_H264_RATECONTROL, VCE_RATE_CONTROL_CONSTANT_BITRATE);
	/// Rate Control: CBR, VBR
	obs_data_set_default_int(settings, AMF_VCE_H264_BITRATE_TARGET, 2500);
	obs_data_set_default_int(settings, AMF_VCE_H264_BITRATE_PEAK, 2500);
	/// Rate Control: Constrained QP
	obs_data_set_default_int(settings, AMF_VCE_H264_QP_MINIMUM, 18);
	obs_data_set_default_int(settings, AMF_VCE_H264_QP_MAXIMUM, 51);
	obs_data_set_default_int(settings, AMF_VCE_H264_QP_IFRAME, 22);
	obs_data_set_default_int(settings, AMF_VCE_H264_QP_PFRAME, 22);
	obs_data_set_default_int(settings, AMF_VCE_H264_QP_BFRAME, 22);
	/// VBV Buffer
	obs_data_set_default_bool(settings, AMF_VCE_H264_USE_CUSTOM_BUFFER_SIZE, false);
	obs_data_set_default_int(settings, AMF_VCE_H264_CUSTOM_BUFFER_SIZE, 2500);
	/// Frame Skipping
	obs_data_set_default_bool(settings, AMF_VCE_H264_FRAME_SKIPPING, false);
}

obs_properties_t* AMFEncoder::VCE_H264_Simple_Encoder::get_properties(void* data) {
	obs_properties* props = obs_properties_create();
	obs_property_t* list;
	obs_property_t* p;

	// Main Properties
	/// Keyframe Interval
	obs_properties_add_int(props, AMF_VCE_H264_KEYFRAME_INTERVAL, obs_module_text(AMF_VCE_H264_KEYFRAME_INTERVAL), 1, 60, 1);

	/// Quality Preset
	list = obs_properties_add_list(props, AMF_VCE_H264_QUALITY_PRESET, obs_module_text(AMF_VCE_H264_QUALITY_PRESET), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_QUALITY_PRESET_SPEED), VCE_QUALITY_PRESET_SPEED);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_QUALITY_PRESET_BALANCED), VCE_QUALITY_PRESET_BALANCED);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_QUALITY_PRESET_QUALITY), VCE_QUALITY_PRESET_QUALITY);

	/// Tuning
	/*list = obs_properties_add_list(props, AMF_VCE_H264_TUNING, obs_module_text(AMF_VCE_H264_TUNING), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_TUNING_DEFAULT), 0);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_TUNING_TWITCH), 1);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_TUNING_YOUTUBE), 2);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_TUNING_RECORDING), 3);*/

	/// Profile
	list = obs_properties_add_list(props, AMF_VCE_H264_PROFILE, obs_module_text(AMF_VCE_H264_QUALITY_PRESET), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_PROFILE_BASELINE), VCE_PROFILE_BASELINE);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_PROFILE_MAIN), VCE_PROFILE_MAIN);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_PROFILE_HIGH), VCE_PROFILE_HIGH);

	/// Rate Control
	list = obs_properties_add_list(props, AMF_VCE_H264_RATECONTROL, obs_module_text(AMF_VCE_H264_RATECONTROL), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_RATECONTROL_CQP), VCE_RATE_CONTROL_CONSTRAINED_QUANTIZATION_PARAMETER);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_RATECONTROL_CBR), VCE_RATE_CONTROL_CONSTANT_BITRATE);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_RATECONTROL_VBR_PEAK_CONSTRAINED), VCE_RATE_CONTROL_VARIABLE_BITRATE_PEAK_CONSTRAINED);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_RATECONTROL_VBR_LATENCY_CONSTRAINED), VCE_RATE_CONTROL_VARIABLE_BITRATE_LATENCY_CONSTRAINED);
	obs_property_set_modified_callback(list, &ratecontrolmethod_modified);

	/// Rate Control: CBR, VBR
	obs_properties_add_int(props, AMF_VCE_H264_BITRATE_TARGET, obs_module_text(AMF_VCE_H264_BITRATE_TARGET), 1, AMFEncoder::VCE_Capabilities::getInstance()->m_AVCCaps.maxBitrate / 1000, 1);
	obs_properties_add_int(props, AMF_VCE_H264_BITRATE_PEAK, obs_module_text(AMF_VCE_H264_BITRATE_PEAK), 1, AMFEncoder::VCE_Capabilities::getInstance()->m_AVCCaps.maxBitrate / 1000, 1);

	/// Rate Control: Constrained QP
	obs_properties_add_int_slider(props, AMF_VCE_H264_QP_MINIMUM, obs_module_text(AMF_VCE_H264_QP_MINIMUM), 1, 51, 1);
	obs_properties_add_int_slider(props, AMF_VCE_H264_QP_MAXIMUM, obs_module_text(AMF_VCE_H264_QP_MAXIMUM), 1, 51, 1);
	obs_properties_add_int_slider(props, AMF_VCE_H264_QP_IFRAME, obs_module_text(AMF_VCE_H264_QP_IFRAME), 1, 51, 1);
	obs_properties_add_int_slider(props, AMF_VCE_H264_QP_PFRAME, obs_module_text(AMF_VCE_H264_QP_PFRAME), 1, 51, 1);
	obs_properties_add_int_slider(props, AMF_VCE_H264_QP_BFRAME, obs_module_text(AMF_VCE_H264_QP_BFRAME), 1, 51, 1);

	/// VBV Buffer
	p = obs_properties_add_bool(props, AMF_VCE_H264_USE_CUSTOM_BUFFER_SIZE, obs_module_text(AMF_VCE_H264_USE_CUSTOM_BUFFER_SIZE));
	obs_properties_add_int(props, AMF_VCE_H264_CUSTOM_BUFFER_SIZE, obs_module_text(AMF_VCE_H264_CUSTOM_BUFFER_SIZE), 1, AMFEncoder::VCE_Capabilities::getInstance()->m_AVCCaps.maxBitrate / 1000, 1);
	obs_property_set_modified_callback(p, &custombuffer_modified);

	/// Frame Skipping
	obs_properties_add_bool(props, AMF_VCE_H264_FRAME_SKIPPING, obs_module_text(AMF_VCE_H264_FRAME_SKIPPING));

	return props;
}

bool AMFEncoder::VCE_H264_Simple_Encoder::ratecontrolmethod_modified(obs_properties_t *props, obs_property_t *property, obs_data_t *settings) {
	// Reset State
	obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_BITRATE_PEAK), false);
	obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_BITRATE_TARGET), false);
	obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_QP_MINIMUM), false);
	obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_QP_MAXIMUM), false);
	obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_QP_IFRAME), false);
	obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_QP_PFRAME), false);
	obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_QP_BFRAME), false);

	switch (obs_data_get_int(settings, AMF_VCE_H264_RATECONTROL)) {
		case VCE_RATE_CONTROL_CONSTRAINED_QUANTIZATION_PARAMETER:
			obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_QP_MINIMUM), true);
			obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_QP_MAXIMUM), true);
			obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_QP_IFRAME), true);
			obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_QP_PFRAME), true);
			obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_QP_BFRAME), true);
			break;
		case VCE_RATE_CONTROL_VARIABLE_BITRATE_PEAK_CONSTRAINED:
		case VCE_RATE_CONTROL_VARIABLE_BITRATE_LATENCY_CONSTRAINED:
			obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_BITRATE_PEAK), true);
		case VCE_RATE_CONTROL_CONSTANT_BITRATE:
			obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_BITRATE_TARGET), true);
			break;
	}
	return true;
}

bool AMFEncoder::VCE_H264_Simple_Encoder::custombuffer_modified(obs_properties_t *props, obs_property_t *property, obs_data_t *settings) {
	obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_CUSTOM_BUFFER_SIZE), false);
	if (obs_data_get_bool(settings, AMF_VCE_H264_USE_CUSTOM_BUFFER_SIZE))
		obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_CUSTOM_BUFFER_SIZE), true);

	return true;
}

void* AMFEncoder::VCE_H264_Simple_Encoder::create(obs_data_t* settings, obs_encoder_t* encoder) {
	try {
		return new AMFEncoder::VCE_H264_Simple_Encoder(settings, encoder);
	} catch (std::exception e) {
		return NULL;
	}
}

void AMFEncoder::VCE_H264_Simple_Encoder::destroy(void* data) {
	delete (static_cast<AMFEncoder::VCE_H264_Simple_Encoder*>(data));
	data = nullptr;
}

bool AMFEncoder::VCE_H264_Simple_Encoder::update(void *data, obs_data_t *settings) {
	return static_cast<AMFEncoder::VCE_H264_Simple_Encoder*>(data)->update(settings);
}

bool AMFEncoder::VCE_H264_Simple_Encoder::encode(void *data, struct encoder_frame * frame, struct encoder_packet * packet, bool * received_packet) {
	return static_cast<AMFEncoder::VCE_H264_Simple_Encoder*>(data)->encode(frame, packet, received_packet);
}

void AMFEncoder::VCE_H264_Simple_Encoder::get_video_info(void *data, struct video_scale_info *info) {
	static_cast<AMFEncoder::VCE_H264_Simple_Encoder*>(data)->get_video_info(info);
}

bool AMFEncoder::VCE_H264_Simple_Encoder::get_extra_data(void *data, uint8_t** extra_data, size_t* size) {
	return static_cast<AMFEncoder::VCE_H264_Simple_Encoder*>(data)->get_extra_data(extra_data, size);
}

//////////////////////////////////////////////////////////////////////////
// Module Code
//////////////////////////////////////////////////////////////////////////
AMFEncoder::VCE_H264_Simple_Encoder::VCE_H264_Simple_Encoder(obs_data_t* settings, obs_encoder_t* encoder) {
	int32_t width, height, fpsNum, fpsDen;
	
	// OBS Settings
	width = obs_encoder_get_width(encoder);
	height = obs_encoder_get_height(encoder);
	video_t *video = obs_encoder_video(encoder);
	const struct video_output_info *voi = video_output_get_info(video);
	fpsNum = voi->fps_num;
	fpsDen = voi->fps_den;

	m_VCE = new VCE(VCE_ENCODER_TYPE_AVC);
	m_VCE->SetMemoryType(VCE_MEMORY_TYPE_HOST);
	switch (voi->format) {
		case VIDEO_FORMAT_NV12:
			m_VCE->SetSurfaceFormat(VCE_SURFACE_FORMAT_NV12);
			break;
		case VIDEO_FORMAT_I420:
			m_VCE->SetSurfaceFormat(VCE_SURFACE_FORMAT_I420);
			break;
		case VIDEO_FORMAT_I444:
			m_VCE->SetSurfaceFormat(VCE_SURFACE_FORMAT_I444);
			break;
		case VIDEO_FORMAT_RGBA:
			m_VCE->SetSurfaceFormat(VCE_SURFACE_FORMAT_RGB);
			break;
	}
	m_VCE->SetUsage(VCE_USAGE_TRANSCODING);

	/// Quality Preset
	m_VCE->SetQualityPreset((VCE_Quality_Preset)obs_data_get_int(settings, AMF_VCE_H264_QUALITY_PRESET));

	/// Profile
	m_VCE->SetProfile((VCE_Profile)obs_data_get_int(settings, AMF_VCE_H264_PROFILE));
	if (width > 1920)
		m_VCE->SetProfileLevel(VCE_PROFILE_LEVEL_51);

	/// Other
	m_VCE->SetScanType(VCE_SCANTYPE_PROGRESSIVE);

	/// Framesize and -rate
	m_VCE->SetFrameSize(std::pair<uint32_t, uint32_t>(width, height));
	m_VCE->SetFrameRate(std::pair<uint32_t, uint32_t>(fpsNum, fpsDen));

	// Dynamic Properties
	/// Tuning
	//switch (0) {
	//	case 1: // Twitch
	//		m_VCE->SetDeblockingFilterEnabled(true);
	//		break;
	//	case 2: // YouTube
	//		m_VCE->SetDeblockingFilterEnabled(true);
	//		break;
	//	case 3: // Recording
	//		m_VCE->SetDeblockingFilterEnabled(true);
	//		break;
	//	case 0:
	//	default:
	//		m_VCE->SetDeblockingFilterEnabled(false);
	//		break;
	//}

	/// Rate Control
	m_VCE->SetRateControlMethod((VCE_Rate_Control_Method)obs_data_get_int(settings, AMF_VCE_H264_RATECONTROL));
	m_VCE->SetFrameSkippingEnabled(obs_data_get_bool(settings, AMF_VCE_H264_FRAME_SKIPPING));
	m_VCE->SetFillerDataEnabled(true);
	m_VCE->SetEnforceHRDEnabled(true);

	/// GOP & VBV Buffer
	m_VCE->SetGOPSize((uint32_t)((double_t)fpsNum / (double_t)fpsDen));
	if (obs_data_get_bool(settings, AMF_VCE_H264_USE_CUSTOM_BUFFER_SIZE)) {
		m_VCE->SetVBVBufferSize((uint32_t)obs_data_get_int(settings, AMF_VCE_H264_CUSTOM_BUFFER_SIZE) * 1000);
	} else {
		uint32_t bufferSize;
		switch (obs_data_get_int(settings, AMF_VCE_H264_RATECONTROL)) {
			case VCE_RATE_CONTROL_CONSTRAINED_QUANTIZATION_PARAMETER:
				bufferSize = (uint32_t)obs_data_get_int(settings, AMF_VCE_H264_QP_MINIMUM) * 500; // Probably not optimal.
				break;
			case VCE_RATE_CONTROL_CONSTANT_BITRATE:
				bufferSize = (uint32_t)obs_data_get_int(settings, AMF_VCE_H264_BITRATE_TARGET);
				break;
			case VCE_RATE_CONTROL_VARIABLE_BITRATE_PEAK_CONSTRAINED:
			case VCE_RATE_CONTROL_VARIABLE_BITRATE_LATENCY_CONSTRAINED:
				bufferSize = (uint32_t)obs_data_get_int(settings, AMF_VCE_H264_BITRATE_PEAK);
				break;
		}
		m_VCE->SetVBVBufferSize(bufferSize * 1000);
	}

	/// Rate Control Parameters
	switch (obs_data_get_int(settings, AMF_VCE_H264_RATECONTROL)) {
		case VCE_RATE_CONTROL_CONSTRAINED_QUANTIZATION_PARAMETER:
			m_VCE->SetMinimumQP((uint8_t)obs_data_get_int(settings, AMF_VCE_H264_QP_MINIMUM));
			m_VCE->SetMaximumQP((uint8_t)obs_data_get_int(settings, AMF_VCE_H264_QP_MINIMUM));
			m_VCE->SetIFrameQP((uint8_t)obs_data_get_int(settings, AMF_VCE_H264_QP_MINIMUM));
			m_VCE->SetPFrameQP((uint8_t)obs_data_get_int(settings, AMF_VCE_H264_QP_MINIMUM));
			m_VCE->SetBFrameQP((uint8_t)obs_data_get_int(settings, AMF_VCE_H264_QP_MINIMUM));
			break;
		case VCE_RATE_CONTROL_CONSTANT_BITRATE:
			m_VCE->SetTargetBitrate((uint32_t)obs_data_get_int(settings, AMF_VCE_H264_BITRATE_TARGET) * 1000);
			m_VCE->SetPeakBitrate((uint32_t)obs_data_get_int(settings, AMF_VCE_H264_BITRATE_TARGET) * 1000);
			break;
		case VCE_RATE_CONTROL_VARIABLE_BITRATE_PEAK_CONSTRAINED:
		case VCE_RATE_CONTROL_VARIABLE_BITRATE_LATENCY_CONSTRAINED:
			m_VCE->SetTargetBitrate((uint32_t)obs_data_get_int(settings, AMF_VCE_H264_BITRATE_TARGET) * 1000);
			m_VCE->SetPeakBitrate((uint32_t)obs_data_get_int(settings, AMF_VCE_H264_BITRATE_PEAK) * 1000);
			break;
	}

	/// Other Settings
	m_VCE->SetNumberOfBPictures(0);
	m_VCE->SetReferenceToBFrameEnabled(false);

	/// Keyframe Interval
	m_VCE->SetIDRPeriod((uint32_t)obs_data_get_int(settings, AMF_VCE_H264_KEYFRAME_INTERVAL) * (uint32_t)((double_t)fpsNum / (double_t)fpsDen));

	/// Motion
	m_VCE->SetQuarterPixelMotionEstimationEnabled(true);
	m_VCE->SetHalfPixelMotionEstimationEnabled(true);

	// Verify
	try {
		AMF_LOG_INFO("Verify Settings:");
		try {
			AMF_LOG_INFO("	Memory Type: %d", m_VCE->GetMemoryType());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Surface Format: %d", m_VCE->GetSurfaceFormat());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Usage: %d", m_VCE->GetUsage());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Quality Preset: %d", m_VCE->GetQualityPreset());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Profile: %d", m_VCE->GetProfile());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Profile Level: %d", m_VCE->GetProfileLevel());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Max LTR Frames: %d", m_VCE->GetMaxLTRFrames());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Scan Type: %d", m_VCE->GetScanType());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Frame Size: %dx%d", m_VCE->GetFrameSize().first, m_VCE->GetFrameSize().second);
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Frame Rate: %d/%d", m_VCE->GetFrameRate().first, m_VCE->GetFrameRate().second);
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Rate Control Method: %d", m_VCE->GetRateControlMethod());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Frame Skipping: %s", m_VCE->IsFrameSkippingEnabled() ? "Enabled" : "Disabled");
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Filler Data: %s", m_VCE->IsFillerDataEnabled() ? "Enabled" : "Disabled");
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Enforce HRD: %s", m_VCE->IsEnforceHRDEnabled() ? "Enabled" : "Disabled");
		} catch (...) {}
		try {
			AMF_LOG_INFO("	GOP Size: %d", m_VCE->GetGOPSize());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	VBV Buffer Size: %d", m_VCE->GetVBVBufferSize());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	VBV Buffer Fullness: %f", m_VCE->GetInitialVBVBufferFullness());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Max AU Size: %d", m_VCE->GetMaximumAccessUnitSize());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	BPic Delta QP: %d", m_VCE->GetBPictureDeltaQP());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Ref BPic Delta QP: %d", m_VCE->GetReferenceBPictureDeltaQP());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Min QP: %d", m_VCE->GetMinimumQP());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Max QP: %d", m_VCE->GetMaximumQP());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	I-QP: %d", m_VCE->GetIFrameQP());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	P-QP: %d", m_VCE->GetPFrameQP());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	B-QP: %d", m_VCE->GetBFrameQP());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Target Bitrate: %d", m_VCE->GetTargetBitrate());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Peak Bitrate: %d", m_VCE->GetPeakBitrate());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Header Insertion Spacing: %d", m_VCE->GetHeaderInsertionSpacing());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	BPic Count: %d", m_VCE->GetNumberOfBPictures());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Deblocking: %s", m_VCE->IsDeblockingFilterEnabled() ? "Enabled" : "Disabled");
		} catch (...) {}
		try {
			AMF_LOG_INFO("	B-Frame Reference: %s", m_VCE->IsReferenceToBFrameEnabled() ? "Enabled" : "Disabled");
		} catch (...) {}
		try {
			AMF_LOG_INFO("	IDR Period: %d", m_VCE->GetIDRPeriod());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Intra-REfresh MBs: %d", m_VCE->GetInfraRefreshMBsPerSlotInMacroblocks());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Slices per Frame: %d", m_VCE->GetNumberOfSlicesPerFrame());
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Half-Pixel Motion: %s", m_VCE->GetHalfPixelMotionEstimationEnabled() ? "Enabled" : "Disabled");
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Quarter-Pixel Motion: %s", m_VCE->GetQuarterPixelMotionEstimationEnabled() ? "Enabled" : "Disabled");
		} catch (...) {}
		try {
			AMF_LOG_INFO("	Temporal Enhancement Layers: %d", m_VCE->GetNumberOfTemporalEnhancementLayers());
		} catch (...) {}
	} catch (...) {}

	m_VCE->Start();
}

AMFEncoder::VCE_H264_Simple_Encoder::~VCE_H264_Simple_Encoder() {
	m_VCE->Stop();
	delete m_VCE;
}

bool AMFEncoder::VCE_H264_Simple_Encoder::update(obs_data_t* settings) {
	return false;
}

bool AMFEncoder::VCE_H264_Simple_Encoder::encode(struct encoder_frame * frame, struct encoder_packet * packet, bool * received_packet) {
	bool retVal = true;
	retVal = m_VCE->SendInput(frame);
	m_VCE->GetOutput(packet, received_packet);
	return retVal;
}

void AMFEncoder::VCE_H264_Simple_Encoder::get_video_info(struct video_scale_info* info) {
	m_VCE->GetVideoInfo(info);
}

bool AMFEncoder::VCE_H264_Simple_Encoder::get_extra_data(uint8_t** extra_data, size_t* size) {
	return m_VCE->GetExtraData(extra_data, size);
}
