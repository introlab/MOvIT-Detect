# System Requirements

* Python 3.6+


Enable SPI, I2C on Raspberry Pi using raspi-config:
```bash
# Run raspi-config
sudo raspi-config

# --> Go to 3 - Interface Options
#   --> Enable P3 - SPI
#   --> Enable P5 - I2C   
```


Install system requirements with:
```bash
# Install python packages
sudo apt-get install python3 python3-venv
```

# Installation

Create a Python virtual environment, enable it and install requirements:
```bash
# Create venv (only once)
python3 -m venv venv
# Activate venv
source venv/bin/activate
# Install all python requirements
pip install -r requirements.txt

```

# Configuration

Make sure your configuration file is updated. Please see example file [config.cfg](config.cfg). 

# Running

```bash
# configure venv
source venv/bin/activate
# Launch
python launcher.py --config=<your config file>
```

 