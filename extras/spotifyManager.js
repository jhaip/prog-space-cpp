/**
 * Original code copied from https://github.com/spotify/web-api-auth-examples/blob/master/authorization_code/app.js
 */

const http = require('http');

var express = require("express"); // Express web server framework
var request = require("request"); // "Request" library
var cors = require("cors");
var querystring = require("querystring");
var cookieParser = require("cookie-parser");

const SERVER_URL = "localhost";

var client_id = process.env.SPOTIFY_CLIENT_ID; // Your client id
var client_secret = process.env.SPOTIFY_CLIENT_SECRET; // Your secret
var redirect_uri = "http://localhost:8888/callback"; // Your redirect uri

var cached_access_token = undefined;
var cached_refresh_token = undefined;

/**
 * Generates a random string containing numbers and letters
 * @param  {number} length The length of the string
 * @return {string} The generated string
 */
var generateRandomString = function (length) {
  var text = "";
  var possible =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

  for (var i = 0; i < length; i++) {
    text += possible.charAt(Math.floor(Math.random() * possible.length));
  }
  return text;
};

var stateKey = "spotify_auth_state";

var app = express();

app
  .use(express.static("./spotify-manager-web-public"))
  .use(cors())
  .use(cookieParser());

app.get("/login", function (req, res) {
  var state = generateRandomString(16);
  res.cookie(stateKey, state);

  // your application requests authorization
  var scope =
    "user-read-private user-read-email user-read-currently-playing user-modify-playback-state";
  res.redirect(
    "https://accounts.spotify.com/authorize?" +
      querystring.stringify({
        response_type: "code",
        client_id: client_id,
        scope: scope,
        redirect_uri: redirect_uri,
        state: state,
      })
  );
});

app.get("/callback", function (req, res) {
  console.log("/callback");

  // your application requests refresh and access tokens
  // after checking the state parameter

  var code = req.query.code || null;
  var state = req.query.state || null;
  var storedState = req.cookies ? req.cookies[stateKey] : null;

  if (state === null || state !== storedState) {
    res.redirect(
      "/#" +
        querystring.stringify({
          error: "state_mismatch",
        })
    );
  } else {
    res.clearCookie(stateKey);
    var authOptions = {
      url: "https://accounts.spotify.com/api/token",
      form: {
        code: code,
        redirect_uri: redirect_uri,
        grant_type: "authorization_code",
      },
      headers: {
        Authorization:
          "Basic " +
          new Buffer(client_id + ":" + client_secret).toString("base64"),
      },
      json: true,
    };

    request.post(authOptions, function (error, response, body) {
      if (!error && response.statusCode === 200) {
        var access_token = body.access_token,
          refresh_token = body.refresh_token;

        cached_access_token = access_token;
        cached_refresh_token = refresh_token;
        console.log("cached_access_token and cached_refresh_token set");

        var options = {
          url: "https://api.spotify.com/v1/me",
          headers: { Authorization: "Bearer " + access_token },
          json: true,
        };

        // use the access token to access the Spotify Web API
        request.get(options, function (error, response, body) {
          console.log(body);
        });

        // we can also pass the token to the browser to make requests from there
        res.redirect(
          "/#" +
            querystring.stringify({
              access_token: access_token,
              refresh_token: refresh_token,
            })
        );
      } else {
        res.redirect(
          "/#" +
            querystring.stringify({
              error: "invalid_token",
            })
        );
      }
    });
  }
});

app.get("/refresh_token", function (req, res) {
  // requesting access token from refresh token
  var refresh_token = req.query.refresh_token;
  var authOptions = {
    url: "https://accounts.spotify.com/api/token",
    headers: {
      Authorization:
        "Basic " +
        new Buffer(client_id + ":" + client_secret).toString("base64"),
    },
    form: {
      grant_type: "refresh_token",
      refresh_token: refresh_token,
    },
    json: true,
  };

  request.post(authOptions, function (error, response, body) {
    if (!error && response.statusCode === 200) {
      var access_token = body.access_token;
      res.send({
        access_token: access_token,
      });
    }
  });
});

function attemptLoginAndTryAgain(retryFunction) {
  console.log("attempting to fetch a new access token using the refresh token");
  var authOptions = {
    url: "https://accounts.spotify.com/api/token",
    headers: {
      Authorization:
        "Basic " +
        new Buffer(client_id + ":" + client_secret).toString("base64"),
    },
    form: {
      grant_type: "refresh_token",
      refresh_token: cached_refresh_token,
    },
    json: true,
  };

  request.post(authOptions, function (error, response, body) {
    if (!error && response.statusCode === 200) {
      cached_access_token = body.access_token;
      console.log("cached_access_token set");
      retryFunction();
    } else {
      console.log(
        "failed getting access token using refresh token, user needs to login again"
      );
      cached_access_token = undefined;
      cached_refresh_token = undefined;
      batch_update([
        { type: "retract", fact: "#0spotify %" },
        { type: "claim", fact: "#0spotify Spotify manager status is LOGIN_REQUIRED_AT_LOCALHOST_8888" },
      ]);
    }
  });
}

function updateCurrentlyPlaying(nRetries) {
  if (nRetries > 1) {
    console.log("MAX NUMBER OF RETRIES REACHED, GIVING UP");
    batch_update([
      { type: "retract", fact: "#0spotify %" },
      { type: "claim", fact: "#0spotify Spotify manager status is MAX_ERRORS_REACHED" },
    ]);
    return;
  }

  var options = {
    url: "https://api.spotify.com/v1/me/player/currently-playing",
    headers: { Authorization: "Bearer " + cached_access_token },
    json: true,
  };

  // use the access token to access the Spotify Web API
  request.get(options, function (error, response, body) {
    if (
      !!error ||
      (response.statusCode !== 200 && response.statusCode !== 204)
    ) {
      console.log("ERROR - update current playing");
      console.log(error);
      console.log(response.statusCode);
      console.log(response.body);
      if (response.statusCode === 401) {
        attemptLoginAndTryAgain(() => updateCurrentlyPlaying(nRetries + 1));
      }
    } else {
      if (response.statusCode === 204) {
        const currentTimeMs = new Date().getTime();
        batch_update([
          { type: "retract", fact: "#0spotify %" },
          { type: "claim", fact: `#0spotify currently playing Spotify song is nothing @ ${currentTimeMs}` },
        ]);
      } else {
        // console.log(body);
        const songTitle = body.item.name.replace(/['"]+/g, '');
        const songArtist = body.item.artists.map((a) => a.name).join(", ").replace(/['"]+/g, '');
        const currentTimeMs = new Date().getTime();
        batch_update([
          { type: "retract", fact: "#0spotify %" },
          {
            type: "claim",
            fact: `#0spotify currently playing Spotify song is "${songTitle}" by "${songArtist}" @ ${currentTimeMs}`,
          },
        ]);
      }
    }
  });
}

function playSpotifyUri(uri, nRetries) {
  if (nRetries > 1) {
    console.log("MAX NUMBER OF RETRIES REACHED, GIVING UP");
    batch_update([
      { type: "retract", fact: "#0spotify %" },
      { type: "claim", fact: "#0spotify Spotify manager status is MAX_ERRORS_REACHED" },
    ]);
    return;
  }

  var jsonBody = {};
  if (uri.includes("track")) {
    jsonBody = {
      uris: [uri], // ex: "spotify:track:232Y5lAjifWnc5NhUn34mz"
    };
  } else {
    jsonBody = { context_uri: uri }; // ex: "spotify:album:04HMMwLmjkftjWy7xc6Bho"
  }
  var options = {
    url: "https://api.spotify.com/v1/me/player/play",
    headers: { Authorization: "Bearer " + cached_access_token },
    method: "PUT",
    json: jsonBody,
  };

  // use the access token to access the Spotify Web API
  request(options, function (error, response, body) {
    if (!!error || response.statusCode !== 200) {
      console.log("ERROR - play song");
      console.log(error);
      console.log(response.statusCode);
      console.log(response.body);
      if (response.statusCode === 401) {
        attemptLoginAndTryAgain(() => playSpotifyUri(uri, nRetries + 1));
      }
      if (response.statusCode === 404) {
        console.log("Device probably needs to be selected");
        batch_update([
          { type: "retract", fact: "#0spotify %" },
          { type: "claim", fact: "#0spotify Spotify manager status is SELECTED_DEVICE_NEEDED" },
        ]);
      }
    } else {
      console.log("successfully started playing song");
      batch_update([
        { type: "retract", fact: "#0spotify %" },
        { type: "claim", fact: "#0spotify Spotify manager status is PLAYING_SONG" },
      ]);
    }
  });
}

function fetch_wishes(queryStr, callback) {
  const options = {
    hostname: SERVER_URL,
    port: 9090,
    path: `/select?query=${encodeURIComponent(
      JSON.stringify([queryStr])
    )}`,
    method: "GET",
  };
  const req = http.request(options, (res) => {
    console.log(`statusCode: ${res.statusCode}`);

    res.on("data", (d) => {
      try {
        const j = JSON.parse(d);
        callback(j);
      } catch (jErr) {
        console.log(jErr);
      }
    });
  });
  req.on("error", (error) => {
    console.error(error);
  });
  req.end();
}

function batch_update(jsonData) {
  const data = JSON.stringify(jsonData);

  const options = {
    hostname: SERVER_URL,
    port: 9090,
    path: "/batch",
    method: "POST",
    headers: {
      "Content-Type": "application/json",
      "Content-Length": data.length,
    },
  };

  const req = http.request(options, (res) => {
    console.log(`statusCode: ${res.statusCode}`);

    res.on("data", (d) => {
      process.stdout.write(d);
    });
  });

  req.on("error", (error) => {
    console.error(error);
  });

  req.write(data);
  req.end();
}

function update() {
  console.log("update");

  fetch_wishes(`$ wish currently Spotify song would be updated`, (results) => {
    console.log("wish current song would be updated");
    console.log(results);
    if (!!results && results.length > 0) {
      updateCurrentlyPlaying();
      batch_update([
        { type: "retract", fact: "$ wish currently Spotify song would be updated" },
      ]);
    }
  });

  fetch_wishes(`$ wish $spotifyUri would be played on Spotify`, (results) => {
    console.log("wish song would be played");
    console.log(results);
    if (!!results && results.length > 0) {
      playSpotifyUri(results[0]["spotifyUri"][1]);
    }
    batch_update([
      { type: "retract", fact: "$ wish $ would be played on Spotify" },
    ]);
  });
}

setInterval(update, 1000);

console.log("Listening on 8888");
app.listen(8888);
