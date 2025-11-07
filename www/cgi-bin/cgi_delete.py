#!/usr/bin/python3

import os
import sys
import json
import http.client

# Get query string to check if we're deleting
query_string = os.environ.get('QUERY_STRING', '')

# Path to uploads directory (relative to CGI script location)
UPLOAD_DIR = os.path.join(os.path.dirname(__file__), 'uploads')

# Server configuration
SERVER_HOST = 'localhost'
SERVER_PORT = 8080

def get_files():
    """Get list of files in uploads directory"""
    try:
        if os.path.exists(UPLOAD_DIR):
            return [f for f in os.listdir(UPLOAD_DIR) if os.path.isfile(os.path.join(UPLOAD_DIR, f))]
        return []
    except Exception as e:
        return []

def send_delete_request(filename):
    """Send DELETE request to the webserv"""
    try:
        conn = http.client.HTTPConnection(SERVER_HOST, SERVER_PORT)
        delete_path = f'/cgi-bin/uploads/{filename}'
        
        conn.request('DELETE', delete_path)
        response = conn.getresponse()
        response_body = response.read().decode('utf-8')
        
        conn.close()
        
        if response.status >= 200 and response.status < 300:
            return True, f"Successfully deleted: {filename} (Status: {response.status})"
        else:
            return False, f"Delete failed: {response.status} - {response_body}"
    except Exception as e:
        return False, f"Error sending DELETE request: {str(e)}"

# Check if we're processing a delete request
if query_string.startswith('delete='):
    filename = query_string.split('=', 1)[1]
    # URL decode the filename
    filename = filename.replace('%20', ' ').replace('%2B', '+').replace('%2F', '/')
    
    success, message = send_delete_request(filename)
    
    # Return JSON response
    sys.stdout.write("Content-Type: application/json\r\n")
    sys.stdout.write("\r\n")
    
    response = {
        "success": success,
        "message": message,
        "filename": filename
    }
    sys.stdout.write(json.dumps(response))
    sys.stdout.flush()
    sys.exit(0)

# Otherwise, display the HTML interface
files = get_files()

sys.stdout.write("Content-Type: text/html\r\n")
sys.stdout.write("\r\n")

html = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>👻 Spooky File Deleter</title>
    <style>
        * {{
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }}

        body {{
            font-family: 'Courier New', monospace;
            background: linear-gradient(135deg, #1a0b2e 0%, #2d1b4e 50%, #1a0b2e 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 20px;
            position: relative;
            overflow-x: hidden;
        }}

        body::before {{
            content: '🎃👻🦇💀🕷️';
            position: fixed;
            top: -50%;
            left: -50%;
            width: 200%;
            height: 200%;
            font-size: 80px;
            opacity: 0.03;
            animation: float 20s infinite linear;
            pointer-events: none;
            z-index: 0;
        }}

        @keyframes float {{
            0% {{ transform: translate(0, 0) rotate(0deg); }}
            100% {{ transform: translate(50px, 50px) rotate(360deg); }}
        }}

        .container {{
            background: rgba(26, 11, 46, 0.9);
            border: 3px solid #ff6b35;
            border-radius: 20px;
            padding: 40px;
            max-width: 700px;
            width: 100%;
            box-shadow: 0 0 40px rgba(255, 107, 53, 0.3), 0 0 80px rgba(138, 43, 226, 0.2);
            position: relative;
            z-index: 1;
        }}

        h1 {{
            color: #ff6b35;
            text-align: center;
            margin-bottom: 10px;
            font-size: 2.5em;
            text-shadow: 0 0 10px rgba(255, 107, 53, 0.5);
            letter-spacing: 2px;
        }}

        .subtitle {{
            color: #9d4edd;
            text-align: center;
            margin-bottom: 30px;
            font-size: 0.9em;
        }}

        .refresh-btn {{
            width: 100%;
            padding: 12px;
            background: linear-gradient(135deg, #9d4edd 0%, #7209b7 100%);
            border: none;
            border-radius: 10px;
            color: white;
            font-size: 1em;
            font-weight: bold;
            cursor: pointer;
            transition: all 0.3s ease;
            font-family: 'Courier New', monospace;
            text-transform: uppercase;
            letter-spacing: 2px;
            margin-bottom: 20px;
        }}

        .refresh-btn:hover {{
            transform: translateY(-2px);
            box-shadow: 0 5px 20px rgba(157, 78, 221, 0.5);
        }}

        .files-list {{
            max-height: 400px;
            overflow-y: auto;
            margin-bottom: 20px;
            border: 2px solid #9d4edd;
            border-radius: 10px;
            background: rgba(157, 78, 221, 0.05);
        }}

        .file-item {{
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 15px;
            border-bottom: 1px solid rgba(157, 78, 221, 0.3);
            transition: all 0.3s ease;
        }}

        .file-item:last-child {{
            border-bottom: none;
        }}

        .file-item:hover {{
            background: rgba(255, 107, 53, 0.1);
        }}

        .file-name {{
            color: #e0aaff;
            font-size: 1em;
            flex: 1;
            word-break: break-all;
        }}

        .delete-btn {{
            padding: 8px 16px;
            background: linear-gradient(135deg, #f94144 0%, #dc2f02 100%);
            border: none;
            border-radius: 8px;
            color: white;
            font-size: 0.9em;
            font-weight: bold;
            cursor: pointer;
            transition: all 0.3s ease;
            font-family: 'Courier New', monospace;
            text-transform: uppercase;
            letter-spacing: 1px;
        }}

        .delete-btn:hover {{
            transform: translateY(-2px);
            box-shadow: 0 3px 15px rgba(249, 65, 68, 0.5);
        }}

        .delete-btn:disabled {{
            background: #666;
            cursor: not-allowed;
            transform: none;
        }}

        .empty-state {{
            padding: 40px;
            text-align: center;
            color: #9d4edd;
            font-size: 1.2em;
        }}

        .message-area {{
            margin-top: 20px;
            padding: 15px;
            background: rgba(26, 11, 46, 0.8);
            border: 2px solid #ff6b35;
            border-radius: 10px;
            color: #e0aaff;
            font-size: 0.9em;
            display: none;
        }}

        .message-area.show {{
            display: block;
        }}

        .message-area.success {{
            border-color: #4ade80;
            color: #4ade80;
        }}

        .message-area.error {{
            border-color: #f87171;
            color: #f87171;
        }}

        ::-webkit-scrollbar {{
            width: 8px;
        }}

        ::-webkit-scrollbar-track {{
            background: rgba(157, 78, 221, 0.1);
            border-radius: 10px;
        }}

        ::-webkit-scrollbar-thumb {{
            background: #9d4edd;
            border-radius: 10px;
        }}

        ::-webkit-scrollbar-thumb:hover {{
            background: #ff6b35;
        }}
    </style>
</head>
<body>
    <div class="container">
        <h1>💀 Spooky File Deleter 🗑️</h1>
        <p class="subtitle">CGI-powered DELETE request sender</p>

        <button class="refresh-btn" onclick="location.reload()">🔄 Refresh File List</button>

        <div class="files-list" id="filesList">
"""

if not files:
    html += '<div class="empty-state">🕸️ No files found in uploads folder</div>'
else:
    for filename in files:
        html += f'''
            <div class="file-item">
                <span class="file-name">{filename}</span>
                <button class="delete-btn" onclick="deleteFile('{filename}')">🗑️ Delete</button>
            </div>
        '''

html += """
        </div>

        <div class="message-area" id="messageArea"></div>
    </div>

    <script>
        function deleteFile(filename) {
            if (!confirm(`Are you sure you want to delete "${filename}"?`)) {
                return;
            }

            const messageArea = document.getElementById('messageArea');
            messageArea.textContent = '⏳ Sending DELETE request...';
            messageArea.className = 'message-area show';

            // Send request to CGI which will send DELETE to webserv
            fetch(`/cgi-bin/cgi_delete.py?delete=${encodeURIComponent(filename)}`)
                .then(response => {
                    console.log('Response status:', response.status);
                    return response.text();
                })
                .then(text => {
                    console.log('Response text:', text);
                    try {
                        const data = JSON.parse(text);
                        if (data.success) {
                            messageArea.textContent = `✅ ${data.message}`;
                            messageArea.className = 'message-area show success';
                            
                            // Reload page after 1.5 seconds to show updated list
                            setTimeout(() => location.reload(), 1500);
                        } else {
                            messageArea.textContent = `❌ ${data.message}`;
                            messageArea.className = 'message-area show error';
                        }
                    } catch (e) {
                        messageArea.textContent = `❌ Invalid response. Check console.`;
                        messageArea.className = 'message-area show error';
                        console.error('Parse error:', e);
                        console.error('Received text:', text.substring(0, 500));
                    }
                })
                .catch(error => {
                    messageArea.textContent = `❌ Error: ${error.message}`;
                    messageArea.className = 'message-area show error';
                    console.error('Fetch error:', error);
                });
        }
    </script>
</body>
</html>
"""

sys.stdout.write(html)
sys.stdout.flush()
