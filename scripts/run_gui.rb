require 'orocos'
require 'vizkit'

include Orocos
Orocos.initialize

sonar_gui = Vizkit.default_loader.SonarWidget

## Execute the main task ##
Orocos.run 'sonar_bvt_sdk::Task' => 'bvt' do
  
  ## Get the task context##
  bvt_sdk = Orocos.name_service.get 'bvt'
  conf_dir = "#{ENV['AUTOPROJ_CURRENT_ROOT']}" << "/drivers/orogen/sonar_bvt_sdk/scripts/ConfigFiles/bvt.yml"

  Orocos.apply_conf_file(bvt_sdk, conf_dir , ['default'] )
  
  ## Configure the task
  bvt_sdk.configure
  
  ## Load current gain and range
  sonar_gui.setRange(bvt_sdk.range)
  sonar_gui.setGain(bvt_sdk.gain * 100)

  ## Set the signals ##
  sonar_gui.connect(SIGNAL('rangeChanged(int)')) do |value|
    bvt_sdk.range = value
  end
  
  sonar_gui.connect(SIGNAL('gainChanged(int)')) do |value|
    bvt_sdk.gain = value * 1.0 / 100
  end
  
  ## Connect SonarWidget with Gemini ##
  bvt_sdk.sonar_samples.connect_to sonar_gui
  
  ## Start the task ##
  bvt_sdk.start
  
  ## Start Vizkit ##
  sonar_gui.show
  Vizkit.exec

end

