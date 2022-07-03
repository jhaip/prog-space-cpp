const http = require('http');
const Particle = require('particle-api-js');
const particle = new Particle();

const SERVER_URL = "localhost";
const LOGIN_INFO = { username: process.env.PARTICLE_USERNAME, password: process.env.PARTICLE_PASSWORD };

var token;

function handleRequest(percentValue) {
    if (!token) {
        console.log("logging in...")
        particle
            .login(LOGIN_INFO)
            .then(data => {
                token = data.body.access_token;
                console.log("got token", token);
                handleRequest(percentValue);
            })
    } else {
        console.log(`Setting value to ${percentValue}`);
        particle
            .callFunction({ deviceId: '200038000747343232363230', name: 'setOpenValue', argument: `${percentValue}`, auth: token })
            .then(data => {
                console.log('Function called succesfully:', data);
            }, err => {
                console.log('An error occurred:', err);
            })
            .catch(err => {
                console.error("ERROR:")
                console.error(err);
                // TODO: make a claim about the error
            });
    }
}

function fetch_wishes(callback) {
  const options = {
    hostname: SERVER_URL,
    port: 9090,
    path: `/select?query=${encodeURIComponent(JSON.stringify(["$ wish plant lamp was at $p percent"]))}`,
    method: 'GET',
  };
  const req = http.request(options, res => {
    console.log(`statusCode: ${res.statusCode}`);

    res.on('data', d => {
      try {
        const j = JSON.parse(d);
        callback(j);
      } catch (jErr) {
        console.log(jErr);
      }
    });
  });  
  req.on('error', error => {
    console.error(error);
  });
  req.end();
}

function batch_update(jsonData) {
  const data = JSON.stringify(jsonData);
  
  const options = {
    hostname: SERVER_URL,
    port: 9090,
    path: '/batch',
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
      'Content-Length': data.length,
    },
  };
  
  const req = http.request(options, res => {
    console.log(`statusCode: ${res.statusCode}`);
  
    res.on('data', d => {
      process.stdout.write(d);
    });
  });
  
  req.on('error', error => {
    console.error(error);
  });
  
  req.write(data);
  req.end();
}

function update() {
  console.log("update");
  fetch_wishes((results) => {
    console.log("got wishes", results);
    if (results.length > 0) {
      batch_update([{"type": "retract", "fact": "$ wish plant lamp was at $ percent"}]);
      const percentValue = parseInt(results[0]["p"][1]);
      handleRequest(percentValue);
      console.log("updated percent value");
    }
    console.log("done update");
  });
}

setInterval(update, 1000);
