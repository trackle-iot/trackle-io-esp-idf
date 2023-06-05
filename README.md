# trackle-io-esp-idf

## API provided

The component provided by this repository implements the following API callable through cloud.

### POST calls
Methods available through POST calls.

#### ResetInputCounter
* Description:
  * Reset counters associated with a particular input.
* Argument format:
  * `<input>`
* Parameters:
  * `<input>`: number of the input whose counters must be reset. 
* Return values:
  * 1:  success;
  * -1: no argument provided.
  
#### SetOutput
* Description:
  * Set the value of a digital output.
* Argument format:
  * `<output>,<value>`
* Parameters:
  * `<output>`: number of the output to be set;
  * `<value>`: value to be set (1 or 0).
* Return values:
  * 1:  success;
  * -1: error.

#### SetInputPublishInterval
* Description:
  * Set input publish interval.
* Argument format:
  * `<input>,<interval>`
* Parameters:
  * `<input>`: number of the input to set interval for;
  * `<interval>`: interval to set in seconds.
* Return values:
  * 1:  success;
  * -1: error.

#### SetInputPublishOnChange
* Description:
  * Set whether to publish input value on change.
* Argument format:
  * `<input>,<publishOnChange>`
* Parameters:
  * `<input>`: number of the input to set on change for;
  * `<publishOnChange>`: 1 to publish on change, 0 otherwise.
* Return values:
  * 1:  success;
  * -1: error.
  
### GET calls
Methods available through GET calls.

#### GetInputStatus
* Description:
  * Get level and counters associated with a particular input.
* Argument format:
  * `<input>`
* Parameters:
  * `<input>`: number of the input to get the status of.
* Returns:
  * JSON object containing the following keys:
    * `status`: the current status of the input (0 or 1);
    * `falling_count`: count of the falling edges;
    * `rising_count`: count of the rising edges;
    * `single_pulse_count`: count of single pulses;
    * `double_pulse_count`: count of double pulses;
    * `long_pulse_count`: count of long pulses.

#### GetInputConfig
* Description:
  * Get configuration of a particular input.
* Argument format:
  * `<input>`
* Parameters:
  * `<input>`: number of the input to take the configuration of;
* Returns:
  * JSON object containing following keys:
    * `publish_interval`: input publish interval in seconds;
    * `publish_changes`: input publish on change (1 or 0).
    
#### GetOutputStatus
* Description:
  * Get actual status of a digital output.
* Argument format:
  * `<output>`
* Parameters:
  * `<output>`: number of output to take the status of;
* Returns:
* JSON object containing following keys:
    * `status`: status of the output (1 or 0).

