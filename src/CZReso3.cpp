// Casio CZ style Reso3 oscillator by Mockba the Borg

#include "plugin.hpp"
#include "MockbaModular.hpp"

struct _Reso3 : _MMOsc {
	_CZWave<float_4> osc;

	float_4 oscStep(float_4 phase, float_4 shape, int wave) override {
		// Calculate the wave step
		return osc.Reso3(phase, shape);
	}
};

struct CZReso3 : Module {
	enum ParamIds {
		_FREQ_PARAM,
		_FINE_PARAM,
		_SHAPE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		_MODF_INPUT,
		_MODS_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		_WAVE_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	_Reso3 osc[4];

	CZReso3() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(_FREQ_PARAM, -54.f, 54.f, 0.f, "Frequency", " Hz", dsp::FREQ_SEMITONE, dsp::FREQ_C4);
		configParam(_FINE_PARAM, -1.f, 1.f, 0.f, "Fine frequency");
		configParam(_SHAPE_PARAM, 0.f, 1.f, 0.f, "Shape");
	}

	void onAdd() override;

	void onReset() override;

	void process(const ProcessArgs& args) override;
};

void CZReso3::onAdd() {
	osc->init();
}

void CZReso3::onReset() {
	onAdd();
}

void CZReso3::process(const ProcessArgs& args) {
	// Get the frequency parameters
	float freqParam = params[_FREQ_PARAM].getValue() / 12.f;
	freqParam += dsp::quadraticBipolar(params[_FINE_PARAM].getValue()) * 3.f / 12.f;
	// Get the shape parameter
	float shapeParam = params[_SHAPE_PARAM].getValue();
	// Iterate over each channel
	int channels = max(inputs[_MODF_INPUT].getChannels(), 1);
	for (int c = 0; c < channels; c += 4) {
		// Get the oscillator
		auto* oscillator = &osc[c / 4];
		float_4 pitch = freqParam;
		// Set the pitch
		pitch += inputs[_MODF_INPUT].getVoltageSimd<float_4>(c);
		oscillator->setPitch(pitch, 1.f);
		// Set the shape
		float_4 shape = shapeParam;
		if (inputs[_MODS_INPUT].isConnected())
			shape *= inputs[_MODS_INPUT].getVoltageSimd<float_4>(c) / 10.f;
		oscillator->setShape(shape);
		// Process and output
		oscillator->process(args.sampleTime);
		outputs[_WAVE_OUTPUT].setVoltageSimd(5.f * oscillator->_Out(), c);
	}
	outputs[_WAVE_OUTPUT].setChannels(channels);
}

struct CZReso3Widget : ModuleWidget {
	CZReso3Widget(CZReso3* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, BGCOLOR)));
		SvgWidget* panel = createWidget<SvgWidget>(Vec(0, 0));
		panel->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CZReso3.svg")));
		addChild(panel);

		// Screws
		addChild(createWidget<_Screw>(Vec(0, 0)));
		addChild(createWidget<_Screw>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Knobs
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 57.0)), module, CZReso3::_FREQ_PARAM));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 68.0)), module, CZReso3::_FINE_PARAM));
		addParam(createParamCentered<_Knob>(mm2px(Vec(5.1, 90.0)), module, CZReso3::_SHAPE_PARAM));

		// Inputs
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 79.0)), module, CZReso3::_MODF_INPUT));
		addInput(createInputCentered<_Port>(mm2px(Vec(5.1, 101.0)), module, CZReso3::_MODS_INPUT));

		// Outputs
		addOutput(createOutputCentered<_Port>(mm2px(Vec(5.1, 112.0)), module, CZReso3::_WAVE_OUTPUT));
	}
};

Model* modelCZReso3 = createModel<CZReso3, CZReso3Widget>("CZReso3");