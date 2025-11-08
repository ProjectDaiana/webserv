#!/usr/bin/php-cgi
<?php
// CGI header
header("Content-Type: text/html");

// Cat treat messages
$treats = [
    "🐟 Here's a delicious salmon treat!",
    "🥩 Fresh tuna just for you!",
    "🦐 Shrimp cocktail, meow!",
    "🐠 A whole mackerel for the good kitty!",
    "🍗 Chicken bits, your favorite!",
    "🥛 Warm milk with a side of love!",
    "🐈 Catnip-infused treats incoming!",
    "🎣 Freshly caught fish from the stream!",
    "🍖 Premium beef strips for the royal cat!",
    "🐭 Mouse-shaped cookies (don't worry, they're fake!)"
];

// Cat ASCII art
$cat_art = [
    <<<EOT
     /\\_/\\  
    ( o.o ) 
     > ^ <
EOT,
    <<<EOT
    |\\___/|
    )     (
   =\\     /=
     )===(
    /     \\
    |     |
   /       \\
   \\       /
EOT,
    <<<EOT
       /\\_/\\
      ( ^.^ )
       > ^ <
      /|   |\\
     (_|   |_)
EOT,
    <<<EOT
    /\\_/\\
   ( o o )
   (  =  )
    )   (
   (     )
EOT
];

// Random cat facts
$cat_facts = [
    "Cats spend 70% of their lives sleeping.",
    "A group of cats is called a 'clowder'.",
    "Cats have over 20 vocalizations, including the meow.",
    "A cat's purr vibrates at a frequency of 25-150 Hz.",
    "Cats can jump up to six times their length!",
    "The first cat in space was French, named Felicette.",
    "Cats have 32 muscles in each ear.",
    "A cat's nose print is unique, like a human fingerprint."
];

// Pick random elements
$selected_treat = $treats[array_rand($treats)];
$selected_art = $cat_art[array_rand($cat_art)];
$selected_fact = $cat_facts[array_rand($cat_facts)];

// Output HTML
echo <<<HTML
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Fishy Treats - KittyServer 🐱</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: 'Courier New', monospace;
            background: linear-gradient(135deg, #FFB6C1 0%, #FFC0CB 50%, #FFB6C1 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 20px;
            position: relative;
            overflow-x: hidden;
        }

        body::before {
            content: '🐱😺🐾😸🐈';
            position: fixed;
            top: -50%;
            left: -50%;
            width: 200%;
            height: 200%;
            font-size: 80px;
            opacity: 0.05;
            animation: float 20s infinite linear;
            pointer-events: none;
            z-index: 0;
        }

        @keyframes float {
            0% { transform: translate(0, 0) rotate(0deg); }
            100% { transform: translate(50px, 50px) rotate(360deg); }
        }

        .container {
            background: rgba(255, 255, 255, 0.95);
            border: 3px solid #FF69B4;
            border-radius: 20px;
            padding: 40px;
            max-width: 600px;
            width: 100%;
            box-shadow: 0 0 40px rgba(255, 105, 180, 0.4), 0 0 80px rgba(255, 182, 193, 0.3);
            position: relative;
            z-index: 1;
            text-align: center;
        }

        h1 {
            color: #FF1493;
            margin-bottom: 20px;
            font-size: 2.5em;
            text-shadow: 0 0 10px rgba(255, 105, 180, 0.5);
            letter-spacing: 2px;
        }

        .cat-art {
            background: rgba(255, 192, 203, 0.2);
            border: 2px solid #FF69B4;
            border-radius: 10px;
            padding: 20px;
            margin: 20px 0;
            font-size: 1.2em;
            color: #8B008B;
            white-space: pre;
            font-family: monospace;
            animation: bounce 2s ease-in-out infinite;
        }

        @keyframes bounce {
            0%, 100% { transform: translateY(0); }
            50% { transform: translateY(-10px); }
        }

        .treat-message {
            font-size: 1.5em;
            color: #FF1493;
            margin: 20px 0;
            font-weight: bold;
            animation: pulse 1.5s ease-in-out infinite;
        }

        @keyframes pulse {
            0%, 100% { transform: scale(1); }
            50% { transform: scale(1.05); }
        }

        .fact-box {
            background: linear-gradient(135deg, rgba(255, 192, 203, 0.3), rgba(255, 182, 193, 0.3));
            border: 2px solid #FF69B4;
            border-radius: 10px;
            padding: 20px;
            margin: 20px 0;
            color: #8B008B;
            font-size: 1em;
        }

        .fact-title {
            font-weight: bold;
            color: #FF1493;
            margin-bottom: 10px;
            font-size: 1.1em;
        }

        .buttons {
            display: flex;
            gap: 15px;
            justify-content: center;
            margin-top: 30px;
            flex-wrap: wrap;
        }

        .button {
            display: inline-block;
            padding: 12px 24px;
            background: linear-gradient(135deg, #FF69B4, #FF1493);
            color: white;
            text-decoration: none;
            border-radius: 10px;
            font-weight: bold;
            transition: all 0.3s ease;
            border: none;
            cursor: pointer;
            font-family: 'Courier New', monospace;
            font-size: 1em;
        }

        .button:hover {
            transform: translateY(-3px);
            box-shadow: 0 5px 20px rgba(255, 105, 180, 0.6);
        }

        .button-secondary {
            background: linear-gradient(135deg, #FFB6C1, #FFC0CB);
            color: #8B008B;
            border: 2px solid #FF69B4;
        }

        .button-secondary:hover {
            background: linear-gradient(135deg, #FFC0CB, #FFB6C1);
        }

        .paw-prints {
            font-size: 2em;
            margin: 20px 0;
            opacity: 0.6;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🐟 Fishy Treats for You! 🐟</h1>

        <div class="cat-art">$selected_art</div>

        <div class="treat-message">$selected_treat</div>

        <div class="paw-prints">🐾 🐾 🐾</div>

        <div class="fact-box">
            <div class="fact-title">💡 Cat Fact of the Day:</div>
            <div>$selected_fact</div>
        </div>

        <div class="buttons">
            <button class="button" onclick="location.reload()">🔄 Get Another Treat</button>
            <a href="/index.html" class="button button-secondary">🏠 Back to Kitty Home</a>
        </div>
    </div>
</body>
</html>
HTML;
?>

