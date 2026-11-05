<script lang="ts">
  import { onMount } from "svelte";
  import * as site from "../typescript/site";
  onMount(() => {
    site.setTitle("Privacy and data handling");
    document.body.style.opacity = "100%";
  });
</script>

<div class="min-h-screen bg-blue-1 text-white font-web flex flex-col items-center px-4 py-10">
  <div class="max-w-2xl w-full">
    <a href="/" class="underline text-sm opacity-80">&larr; Homepage</a>
    <h1 class="text-4xl font-bold mt-3 mb-1">vNAAATS - privacy and data handling</h1>
    <div class="opacity-80 mb-6">Last updated 23 October 2026</div>

    <p class="p-4 mb-6 border-2 border-blue-10 bg-blue-3"><b>vNAAATS is an independent, community project. It is not run, owned or endorsed by VATSIM.</b> It is for flight simulation only and must never be used for real-world navigation or air traffic control.</p>

    <h2 class="text-2xl font-bold mb-2">Who runs it</h2>
    <p class="mb-6 opacity-90">vNAAATS Version 2 is maintained by Gary Thomas (original vNAAATS by Andrew Ogden). Gary Thomas is the person responsible for the personal data described on this page. Questions, corrections and requests about your data: <a class="underline" href="mailto:info@vnaaats.com">info@vnaaats.com</a> (or <a class="underline" href="https://github.com/garythms/vatsim-naaats-v2/issues">GitHub issues</a>).</p>

    <h2 class="text-2xl font-bold mb-2">What this site and server handle</h2>
    <ul class="list-disc pl-6 mb-6 space-y-2 opacity-90">
      <li><b>Public VATSIM data.</b> Pilot callsigns, positions and flight plans, and online controller callsigns and frequencies, are read from VATSIM's public data feeds. Oceanic clearance information comes from natTrak (nattrak.vatsim.net).</li>
      <li><b>Flight data that controllers enter.</b> Level, Mach, track, route changes, SELCAL, notes and similar are stored on the server with the name and callsign of the controller who last changed them, and the time. Flights are removed some time after they stop being updated.</li>
      <li><b>Sign-in (Flight Data Display).</b> If you sign in with VATSIM, the server receives your name, VATSIM ID (CID) and controller rating, which it uses to decide whether you may make changes. If a password is used instead, you enter the name you want shown. Your sign-in lives in a session kept in the server's memory for up to 12 hours; it is dropped when you sign out or the server restarts.</li>
      <li><b>Personal plugin keys and passwords.</b> A certified (endorsed) Gander Oceanic controller can request their own key and password from the Gander Oceanic website. The server stores your CID, your name, when the key was made and when it expires (90 days), when it was last used, and a one-way hash of the key and of the password, so a copy of the database cannot reveal either. Whether you are certified is checked against the public Gander Oceanic roster (ganderoceanic.ca/api/roster), which also lists the oceanic controllers of VATSIM UK and Virtual New York ARTCC. Only oceanic controllers on that roster are checked, and your key stops working if you leave it.</li>
      <li><b>Getting an API key (vnaaats.com/get-key).</b> When you ask for a key the server stores your CID, the name and email address you type, your IP address at that moment, and when the key was made and when it expires (90 days). This is kept so that misuse of a key can be followed up with you, the VATSIM supervisors and your division. It is not shown to other controllers or shared otherwise. It stays with your key record until that record is removed.</li>
      <li><b>FDD for Windows (the Windows app, optional).</b> It opens this website in its own window and, only when you ask it to or when you choose your position, it can check and replace the plugin DLL and ICAO_Airlines.txt in the folder you choose, write your key into your own vnaaats.cfg, and start EuroScope and VACS from the paths you pick. The folder and program paths are kept on your own computer. If you choose "Remember me", your sign-in name and password are kept in the Windows Credential Manager on your computer, not on our server, and you can forget them from the sign-in screen. Links to natTrak and similar sites open in your normal browser. The app sends nothing to us beyond what the website itself does.</li>
      <li><b>The EuroScope plugin.</b> It sends flight plan changes, which aircraft you have selected and are tracking (kept in the server's memory for a few minutes so your aircraft can be re-tracked after a crash), and your controller callsign to this server, using your key.</li>
      <li><b>CPDLC messages.</b> Datalink messages sent through the server's CPDLC station (via the Hoppie network) are kept on the server so controllers can see the conversation. The Hoppie logon code you type is held in the server's memory only while you are signed in. It is never written to disk.</li>
      <li><b>Position times for Concorde.</b> Times typed in for Concorde flights are kept for 24 hours.</li>
      <li><b>Cookie and browser storage.</b> One sign-in session cookie, used only to keep you signed in. Your browser also stores a few settings on your own device (for example strips you have hidden, the time-window filter and whether you have been sent to sign in to natTrak). There are no advertising, analytics or tracking cookies.</li>
      <li><b>Server logs.</b> Like any web server, requests (including IP address, time and page) are written to the ordinary server logs for running and fixing the service. They are not shared or used for anything else.</li>
      <li><b>Discord notices.</b> The server and the build system post plain notices to a Discord channel when a new version is released. They contain no personal data.</li>
    </ul>

    <h2 class="text-2xl font-bold mb-2">Services the server talks to</h2>
    <p class="mb-6 opacity-90">VATSIM (public data feed and, once approved, VATSIM Connect sign-in), natTrak, the Hoppie ACARS network (CPDLC), the Gander Oceanic website (roster check) and GitHub (downloads of the plugin). Each has its own privacy policy.</p>

    <h2 class="text-2xl font-bold mb-2">What we do not do</h2>
    <ul class="list-disc pl-6 mb-6 space-y-2 opacity-90">
      <li>We do not sell or give your data to anyone, and we do not use it for advertising or tracking.</li>
      <li>We do not collect your VATSIM password or any other service's password. VATSIM sign-in takes place on VATSIM's own site.</li>
    </ul>

    <h2 class="text-2xl font-bold mb-2">Why the data is used, and how long it is kept</h2>
    <ul class="list-disc pl-6 mb-6 space-y-2 opacity-90">
      <li><b>Why:</b> only to run the tool: to show flight data, to let controllers make and share changes, and to make sure only the right people can. The basis is our legitimate interest in running a community flight simulation service, and your agreement when you sign in or ask for a key.</li>
      <li><b>Flights</b> are removed some time after they stop being updated. <b>Sign-in sessions</b> end after 12 hours. <b>Concorde times</b> are kept for 24 hours. <b>Personal keys</b> last 90 days and are withdrawn if you leave the roster; the record is kept until you ask for it to be removed. <b>CPDLC messages</b> are kept on the server until they are removed.</li>
      <li><b>Where:</b> the server is hosted in Germany (Hetzner). Because VATSIM is a worldwide network, your name and CID can be seen by other controllers using the display. Data is not otherwise sent outside the services listed above.</li>
    </ul>

    <h2 class="text-2xl font-bold mb-2">Your rights and choices</h2>
    <p class="mb-3 opacity-90">Whatever country you are in, you can ask us to tell you what is held about you, correct it, delete it, limit how it is used, or give you a copy, and you can object to how it is used. In the UK and the EU these rights come from the UK GDPR and the EU GDPR; we apply them to everyone. Email <a class="underline" href="mailto:info@vnaaats.com">info@vnaaats.com</a> and quote your VATSIM ID. We aim to reply within one month.</p>
    <p class="mb-6 opacity-90">You can sign out at any time. If you are not happy with how we handle your data you can also complain to your local data protection authority (in the UK, the Information Commissioner's Office, ico.org.uk).</p>

    <p class="text-sm opacity-80"><a class="underline" href="/">Back to vNAAATS</a></p>
  </div>
</div>
