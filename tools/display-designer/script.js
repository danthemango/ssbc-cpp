const canvas = document.getElementById("canvas");
canvas.width = 960;
canvas.height = 960;
const ctx = canvas.getContext("2d");

let circuitMode = false;

// num pixels on canvas per segment on display
const pixelSize = 4;
// number of pixels between display segments in the canvas
const padding = 4;
// pixels per display row/column
const numPixels = 8;
// number of display rows and columns
const numDisplays = 16;

// filling with some of the displays I've already designed
// will probably change later
const cache = {
    "00": "1824424242422418",
    "01": "000c14040404041e",
    "03": "001c041c04043c00",
    "02": "0018240408103c00",
    "04": "002424241c040404",
    "05": "003c20201c04043c",
    "06": "001e203e22221c00",
    "07": "003e040810202000",
    "08": "0018242418242418",
    "09": "00182424241c0418",
    "0a": "001824427e424200",
    "0b": "3824243824243800",
    "0c": "001c202020201c00",
    "0d": "003c222222223c00",
    "0e": "00003c2038203c00",
    "0f": "00003c2038202000"
};

// returns the hex code for an empty table
function getEmptyTableHex() {
  let th = '';
  while(th.length < numPixels*2) {
    th += '00';
  }
  return th
}

// fetches the table hex string and updates the cache
function updateCacheFromTable() {
  const ih = getInputHex();
  const th = getTableHex();
  cache[ih] = th
  console.log(cache);
}

// takes hex string from cache and updates the table with it
function updateTableFromCache() {
  const ih = getInputHex();
  if(!cache[ih]) {
    // set table to 0's if not already set
    let th = getEmptyTableHex();
    cache[ih] = th;
    setTableFromHex(th);
    return;
  }
  let th = cache[ih];
  setTableFromHex(th);
}

  // returns a hex representation from a given binary string
  function binary2hex(n) {
    let s = parseInt(n, 2).toString(16);
    if(s.length == 1) {
      s = '0' + s;
    }
    return s;
  }

  // display row -> 8-bit binary string
  function row2binary(n) {
    const tbody = document.querySelector('#lights>table>tbody');
    const row = tbody.children[n].children;
    const rowArray = Array.from(row);
    let res = rowArray.map(isOn).map(i => i ? '1' : '0').join('');
    while(res.length < 8) {
      res = '0' + res;
    }
    return res;
  }
  
  // returns a hex code for a given display table row
  function getRowHex (n) {
    return binary2hex(row2binary(n));
  }
  
  // returns a hex code from the current display table
  // padded to 2 digits if necessary
  function getTableHex () {
    const h = getHeight();
    let result = '';
    for(let y = 0; y < h; y++) {
      result += getRowHex(y);
    }
    return result;
  }

  // 2-bit hex string -> 8-bit binary string
  function hex2binary(n) {
    let result = parseInt(n,16).toString(2);
    while(result.length < 8) {
      result = '0' + result;
    }
    return result;
  }

  // update table row n for hex value
  function setRowFromHex(n, hex) {
    const tbody = document.querySelector('#lights>table>tbody');
    const row = tbody.children[n].children;
    const rowArray = Array.from(row);
    const bs = hex2binary(hex); // binary string
    // update each switch from the right to the left,
    // since we don't know how many switches there are in the table
    // (up to 8)
    let bi = bs.length-1;
    let li = row.length-1;
    while(bi >= 0 && li >= 0) {
      const sw = row[li];
      if(bs[bi] == '1') {
        switchOn(sw);
      } else {
        switchOff(sw);
      }
      --bi;
      --li;
    }
  }
  
  // splits table hex into 2-digit parts of each row
  function getRowHexArrFromTableHex(th) {
    let result = [];
    for(let i = 0; i < th.length; i+=2) {
      // hex for one switch
      let swHex = th[i] + th[i+1];
      result.push(swHex);
    }
    return result;
  }

  // updates the display table from 
  function setTableFromHex (th) {
    // each row corresponds to 2 hex digits of the input
    const arr = getRowHexArrFromTableHex(th);
    for(let i = 0; i < arr.length; ++i) {
      // hex for one switch
      const swHex = arr[i];
      setRowFromHex(i, swHex);
    }
  }

  // for circuit mode, removes 'select' class from all lights
function unselectAllDisplay() {
  const displayLights = Array.from(document.querySelectorAll('#lights td'));
  displayLights.forEach(el => {
    if(el.classList.contains('selected')) {
      el.classList.remove('selected');
    }
  })
}

// get the binary input -> bit sequences object for the specified row and column
function getPixelObj(col, row) {
  const outObj = {}
  for(let [inputHex, tableHex] of Object.entries(cache)) {
    const inputBinary = hex2binary(inputHex);
    const thArr = getRowHexArrFromTableHex(tableHex);
    const rowBinary = thArr.map(i => hex2binary(i))[row];
    const result = rowBinary[rowBinary.length-1-col];
    outObj[inputBinary] = result;
  }
  console.log(outObj)
  return outObj;
}

// switches a light from on to off
function onDisplaySwitch(el) {
  if(circuitMode) {
    // unselect every other light, then select this light
    unselectAllDisplay();
    el.classList.add('selected');
    // add get circuit sequence, and add it to the circuit input
    const row = el.dataset['row'];
    const col = el.dataset['col'];
    const outObj = getPixelObj(col,row);
    document.querySelector('input#circuit-out').value = getCircuitSequences(outObj);
    return;
  }

  if(el.classList.contains('on')) {
    el.classList.remove('on');
  } else {
    el.classList.add('on');
  }
  
  updateCacheFromTable();
  updateCurrentCanvasDisplay();
}

function switchOn(el) {
  if(!el.classList.contains('on')) {
    el.classList.add('on');
  }
}

function switchOff(el) {
  if(el.classList.contains('on')) {
    el.classList.remove('on');
  }
}

// returns an array of light elements
function getLights() {
  return Array.from(document.querySelectorAll('#lights td'));
}

function getInputLights() {
  return Array.from(document.querySelectorAll('#input-lights td'));
}

// returns a light element
function getNewLight(c,r) {
  const td = document.createElement('td');
  td.dataset['col'] = c;
  td.dataset['row'] = r;
  td.onclick = () => onDisplaySwitch(td);
  return td;
}

// returns a row of n columns
function getNewRow(r) {
  const row = document.createElement('tr');
  const width = getWidth();
  for(let c = 0; c < width; c++) {
    row.appendChild(getNewLight(c,r));
  }
  return row;
}

// returns a new table
function getNewTable() {
  const table = document.createElement('table');
  const tbody = document.createElement('tbody');
  table.appendChild(tbody);
  const height = getHeight();
  for(let r = 0; r < height; r++) {
    tbody.appendChild(getNewRow(r));
  }
  return table;
}

// switches current display table with a new table
function createTable() {
  const div = document.querySelector('div#lights');
  const currentTable = document.querySelector('div#lights>table');
  div.removeChild(currentTable);
  div.appendChild(getNewTable());
}

function getWidth() {
  return parseInt(document.querySelector('#options #width').value);
}

function getHeight() {
  return parseInt(document.querySelector('#options #height').value);
}

// returns a padded 8-bit binary string, as long as it is not too large or invalid
function getPadded8bit(val) {
  if(val.length > 8) {
    return;
  }
  for(let i = 0; i < val.length; i++) {
    if(val[i] != '0' && val[i] != '1') {
      return;
    }
  }
  let result = "";
  for(let i = val.length; i < 8; i++) {
    result += '0';
  }
  result += val;
  return result;
}

// update to run after change of binary input
function onInputBinary() {
  const el = document.querySelector('#binary');
  let switchBinary = getPadded8bit(el.value);
  const inputLights = document.querySelectorAll('#input-lights td');
  if(switchBinary && inputLights) {
    for(let i = 0 ; i < 8; i++) {
      if(switchBinary[i] == '1') {
        switchOn(inputLights[i]);
      } else {
        switchOff(inputLights[i]);
      }
    }
  }
  
  let [x,y] = binary2displayCoords(switchBinary);
  drawBorder(x,y);
  updateTableFromCache();
  updateCurrentCanvasDisplay();
}

// returns true if the switch is on
function isOn(light) {
  return light.classList.contains('on');
}

// returns a binary string from input lights
function getInputBinary() {
  const lights = getInputLights();
  let result ='';
  for(let i = 0; i < 8; i++) {
    const light = lights[i];
    if(isOn(light)) {
      result += '1';
    } else {
      result += '0'
    }
  }
  return result;
}

function getInputHex() {
  const bs = getInputBinary();
  return binary2hex(bs);
}

// converts binary string to display column and row of the chosen display
function binary2displayCoords(s) {
  let n = Number.parseInt(s,2);
  let x = n % 16;
  let y = (n-x) / 16;
  return [x,y]
}

function displayCoords2Hex(x,y) {
  const n = 16*y+x;
  let bs = n.toString(16);
  while(bs.length < 2) {
    bs = '0' + bs;
  }
  return bs;
}

// updates binary input from input switches
function onInputSwitch(el) {
  if(circuitMode) {
    return;
  }

  if(el.classList.contains('on')) {
    el.classList.remove('on');
  } else {
    el.classList.add('on');
  }
  const switchBinary = getInputBinary();
  document.querySelector('#binary').value = switchBinary;
  let [x,y] = binary2displayCoords(switchBinary);
  drawBorder(x, y);
  updateTableFromCache();
  updateCurrentCanvasDisplay();
}

// places the rgbaArray (4-part array) onto imagedata
function setPixelIID(imageData, i, color) {
  // red green blue alpha
  const [r,g,b,a] = color;

  imageData.data[i + 0] = r;
  imageData.data[i + 1] = g;
  imageData.data[i + 2] = b;
  imageData.data[i + 3] = a;
}

// insert image data pixel at x,y to color
function setPixelXYID(imageData, x, y, color) {
  // width of the display
  const w = pixelSize * numPixels;
  // there are 4 bits per pixel in image data: rgba
  const i = (y * w + x)*4;
  const rgbaArray = color;
  setPixelIID(imageData, i, color);
}


// setting one pixel-sized square
function drawPixelArea(imageData, ax, ay, color) {
  const ps = pixelSize;
  // x and y offset
  let ox = ax*ps;
  let oy = ay*ps;
  for (let y = oy; y < ps+oy; y++) {
    for(let x = ox; x < ps+ox; x++) {
      setPixelXYID(imageData, x, y, color);
    }
  }
}

function initCanvas() {

  function getImageData(c,r) {
    const size = pixelSize * numPixels;

    const imageData = ctx.createImageData(size, size);

    // Iterate through every pixel
    for (let y = 0; y < size; y++) {
      for(let x = 0; x < size; x++) {
        setPixelXYID(imageData, x, y, [220,220,220,255]);
      }
    }
    
    return imageData;
  }

  // draw 16x16 displays
  const dispRows = 16;
  const dispCols = 16;
  for(let ri = 0; ri < dispRows; ri++) {
    for(let ci = 0; ci < dispCols; ci++) {
      const inputHex = displayCoords2Hex(ci,ri)
      let tableHex = getEmptyTableHex();
      if(cache[inputHex]) {
        tableHex = cache[inputHex];        
      }
      
      updateCanvasDisplay(ci,ri, tableHex);
    }
  }
}

// set the imagedata for a given row from the hexadecimal code
function setImageDataRowFromHex(imageData, y, hex) {
  const bs = hex2binary(hex); // binary string
  // update each switch from the right to the left,
  // since we don't know how many switches there are in the table
  // (up to 8)
  let width = 8;
  let bi = bs.length-1;
  let x = width-1;

  while(bi >= 0 && x >= 0) {
    const bit = bs[bi]
    if(bit == '1') {
      drawPixelArea(imageData, x, y, [0, 0, 0, 255]); 
    } else {
      drawPixelArea(imageData, x, y, [220,220,220,255]); 
    }
    --bi;
    --x;
  }
}

// returns canvas image data from display table
function getDisplayImageData(tableHex) {
  const s = pixelSize*numPixels;
  const imageData = ctx.createImageData(s,s);
  
  const arr = getRowHexArrFromTableHex(tableHex);
  for(let i = 0; i < arr.length; i++) {
    setImageDataRowFromHex(imageData, i, arr[i])
  }
  return imageData;
}

// returns the top-left pixel position of the display at row r, col c
function getDisplayPos(x,y) {
  const p = padding;
  const s = pixelSize * numPixels; // pixel size (width/height) of display section
  return [p+(p+s)*x,p+(p+s)*y];
}

// update the canvas display at col,row
function updateCanvasDisplay(col,row, tableHex) {
  const [x,y] = getDisplayPos(col, row);
  const imageData = getDisplayImageData(tableHex);
  ctx.putImageData(imageData, x, y);
}

// update the current display in the canvas
function updateCurrentCanvasDisplay() {
  const bs =  getInputBinary();
  let [col,row] = binary2displayCoords(bs);
  const tableHex = getTableHex();
  updateCanvasDisplay(col,row, tableHex);
}

// puts image data in the x and y position for a given display item 
function putImageDataForDisplay(imageData, col, row) {
  const [x,y] = getDisplayPos(col, row);
  ctx.putImageData(imageData, x, y);
}

let previousBorder;
function drawBorder(x,y) {
  if(previousBorder) {
    const [prevX, prevY] = previousBorder;
    drawBorderColor(prevX,prevY,"white");
  }
  previousBorder = [x,y];
  drawBorderColor(x,y,"black");
}

function eraseBorder(x,y) {
  drawBorderColor(x,y,"white");
}

// draw a border around the display at position x,y
function drawBorderColor(x,y, color = "black") {
  const ps = pixelSize;
  const np = numPixels;
  const pd = padding;
  const s = ps*np; // num pixels in a display
  // the space between the start of each segment
  const w = ps*np + pd; 
  ctx.beginPath();
  ctx.lineWidth = "2";
  ctx.strokeStyle=color;
  ctx.rect(pd-1+x*w, pd-1+y*w, s+2, s+2);
  ctx.stroke();
}

// returns a boolean array of row n of the active display
function row2array(n) {
  return getLights().map(isOn);
}

// toggle circuit-mode
function onCircuitSelect() {
  circuitMode = !circuitMode;
  const inputLights = Array.from(document.querySelectorAll('#input-lights td'));
  const displayLights = Array.from(document.querySelectorAll('#lights td'));
  if(circuitMode) {
    // enable circuit mode
    document.querySelector('input#binary').setAttribute('disabled',true);
    document.querySelector('input#width').setAttribute('disabled',true);
    document.querySelector('input#height').setAttribute('disabled',true);
    inputLights.forEach(el => el.classList.add('circ'));
    displayLights.forEach(el => el.classList.add('circ'));
    document.querySelector('#circ-pixel-ins').classList.remove('hide');
  } else {
    // disable circuit mode
    document.querySelector('input#binary').removeAttribute('disabled');
    document.querySelector('input#width').removeAttribute('disabled');
    document.querySelector('input#height').removeAttribute('disabled');
    inputLights.forEach(el => el.classList.remove('circ'));
    displayLights.forEach(el => el.classList.remove('circ'));
    document.querySelector('#circ-pixel-ins').classList.add('hide');
    unselectAllDisplay();
    document.querySelector('input#circuit-out').value = '';
  }

}

// show the help alert
function onCircuitHelp() {
  alert(
    `
Click the checkmark to enable circuit-mode, which uses the 8-bit
input sequences and creates specifications for a minified circuit
for the display pixel.

The specifications are a series of 8-digit sequences separated
by bars ('|'), where:
- '1' means the pin of this 8-bit input position needs to be on
- '0' means the pin needs to be off
- 'x' means the pin may be anything, on or off
- '|' means "or"

note:
- if the only thing you see is '0', then it means this pixel is off at all times
- if the only thing you see is 'xxxxxxxx', then it means this pixel is on at all times
`
  );
}

// returns a set of minimal circuits needed to cover
// all output values, or 0 if there are none
function getCircuitSequences(outObj) {
    // returns the maximum bit-container for a given input string
    // else return 0 if none exists
    /*
        note: a 'bit-container' is a sequence of 0's, 1's, and x's
        a valid bit container results in a 1 in the output object for every input string
        which can be created by replacing the x's with either a 1 or a 0
    */
    const maxContainerCache = {};
    function getMaxContainer(inString, outObj) {
        /*
            set the maxContainer to the input string,
            for each position which has a 1 or a 0:
                create newContainer with an x added in this position
                if it's a valid container
                    recursively run this function again with the new container
                    if that is also a valid container, set it to newContainer
                    if newContainer > maxContainer
                        maxContainer = newContainer
            return maxContainer
        */
        if (maxContainerCache[inString]) {
            return maxContainerCache[inString];
        }
        let maxContainer = inString;
        if (!isValidContainer(maxContainer, outObj)) {
            return 0;
        }
        for (let i = 0; i < inString.length; i++) {
            if (inString[i] == '0' || inString[i] == '1') {
                let newContainer = getReplacedChar(inString, i, 'x');
                if (isValidContainer(newContainer, outObj)) {
                    let subMaxContainer = getMaxContainer(newContainer, outObj);
                    if (subMaxContainer != 0) {
                        newContainer = subMaxContainer;
                    }
                    if (isLargerContainer(newContainer, maxContainer)) {
                        maxContainer = newContainer;
                    }
                }
            }
        }
        maxContainerCache[inString] = maxContainer;
        return maxContainer;
    }

    // returns a new string from input string with the character at i replaced with c
    function getReplacedChar(str, i, c) {
        return str.substring(0, i) + c + str.substring(i + 1);
    }

    // checks if the output for all input string permutations equal 1 in the output object
    // with the x's in inString replaced by either 0's or 1's
    function isValidContainer(inString, outObj) {
        let hasX = false;
        for (let i = 0; i < inString.length; i++) {
            if (inString[i] == 'x') {
                hasX = true;
                let subString0 = getReplacedChar(inString, i, '0');
                subString0[i] = '0';
                if (!isValidContainer(subString0, outObj)) {
                    return false;
                }
                let subString1 = getReplacedChar(inString, i, '1');
                if (!isValidContainer(subString1, outObj)) {
                    return false;
                }
            }
        }

        if (!hasX) {
            if (!outObj[inString]) {
                return true;
            } else {
                return outObj[inString] == '1';
            }
        }

        return true;
    }

    // returns true if containerA has more x's than containerB
    function isLargerContainer(containerA, containerB) {
        const aSize = Array.from(containerA).filter(i => i == 'x').length;
        const bSize = Array.from(containerB).filter(i => i == 'x').length;
        return aSize > bSize;
    }

    // returns true if the input binary string is contained by one of the strings
    // in the container array
    function isInContainers(inString, containers) {
        for (let container of containers) {
            if (isInContainer(inString, container)) {
                return true;
            }
        }
        return false;
    }

    // returns true if the input binary string is contained by one of the strings in the container 
    function isInContainer(inString, container) {
        for (let i = 0; i < inString.length; i++) {
            if (container[i] != 'x' && container[i] != inString[i]) {
                return false;
            }
        }
        return true;
    }

    // returns an array of the most minimal circuits needed to cover
    // all output values, or 0 if there are none
    function getBitContainers(outObj) {
        // for each binary input string, get the maximum container for it,
        // add it to a map to ensure this container is uniquely identified,
        // and return all max containers for all bits
        const resObj = {};
        const containers = [];
        for (let inString of Object.keys(outObj)) {
            if (!isInContainers(inString, containers)) {
                const maxContainer = getMaxContainer(inString, outObj);
                if (maxContainer != 0) {
                    resObj[maxContainer] = '1';
                }
            }
        }
        let result = Object.keys(resObj);
        if (result.length == 0) {
            return ['0']
        } else {
            return result;
        }
    }

    return getBitContainers(outObj).join('|');
}

function main() {
  console.clear();
  createTable();
  onInputBinary();

  // enable light switches
  getLights().forEach(el => el.onclick = () => onDisplaySwitch(el))

  // enable input lights switches
  getInputLights().forEach(el => el.onclick = () => onInputSwitch(el))

  // enable updating the canvas with every click
  const binaryInput = document.querySelector('#binary');
  binaryInput.oninput = () => onInputBinary();

  initCanvas();
  updateCurrentCanvasDisplay();
  drawBorder(0,0);
}

main();