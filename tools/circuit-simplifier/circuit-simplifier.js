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

function getRandomSequence() {
    const numBits = 20;

    // get an array of all binary permutations of size n
    function getBinaryPermutations(n) {
        if (n == 1) {
            return ['0', '1'];
        }

        let result = [];
        for (let bp of getBinaryPermutations(n - 1)) {
            result.push(bp + '0');
            result.push(bp + '1');
        }
        return result;
    }

    // returns a random binary output of size n
    function getRandomPermutation(n) {
        let result = '';
        while (n > 0) {
            if (Math.random() > 0.5) {
                result += '1'
            } else {
                result += '0';
            }
            --n;
        }
        return result;
    }

    // create object to store binary input string -> output (on or off)
    const outObj = {}
    const inStrings = getBinaryPermutations(numBits);
    const outString = getRandomPermutation(inStrings.length);
    for (let i = 0; i < inStrings.length; i++) {
        const inString = inStrings[i];
        const outBit = outString[i];
        outObj[inString] = outBit;
    }

    // console.log(outObj)
    let res = getCircuitSequences(outObj);
    console.log(res);
    return res;
}

getRandomSequence()