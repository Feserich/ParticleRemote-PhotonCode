# ParticleRemote-PhotonCode
The [Particle Photon](https://www.particle.io/products/hardware/photon-wifi-dev-kit) is a Wi-Fi connected microcontroller.
Together with the related [Android app](https://github.com/Feserich/ParticleRemote-Android), it's possible to  remote control a little home automation with multiple Particle devices.


The [Android app](https://github.com/Feserich/ParticleRemote-Android) together with the Particle board can do following tasks: 
* Set the temperature for the _Honeywell HR-20_ radiator regulator (memory manipulation over programming/debugging interface. 
More Infos [here](http://symlink.dk/projects/rondo485/).).
* Switch some relays.
* Read the current temperature and moisture with a DHT22 sensor.
* Show the temperature and moisture history in a chart (over the last 6 days).
* Add and control multiple Particle devices.

**Here is a wiring example:**

<img src="https://github.com/Feserich/ParticleRemote-PhotonCode/blob/master/Wiring%20Example/Photon%20Wiring_bb.png" width="420"/>
