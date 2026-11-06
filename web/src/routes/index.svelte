<script lang="ts">
import { onMount } from "svelte";
 
import * as site from "../typescript/site";

let pagesOpen: boolean = false;
let heroVideo: HTMLVideoElement;
let heroPlaying: boolean = true;
function toggleHero() {
    if (!heroVideo) return;
    if (heroVideo.paused) { heroVideo.play(); heroPlaying = true; } else { heroVideo.pause(); heroPlaying = false; }
}
const pages: { href: string; label: string }[] = [
    { href: "/access", label: "Get access" },
    { href: "/fdd", label: "Flight Data Display" },
    { href: "/windows", label: "FDD for Windows" },
    { href: "/download", label: "Downloads" },
    { href: "/guide", label: "Install guide" },
    { href: "/manual", label: "User manual" },
    { href: "/videos", label: "How To Use - Videos" },
    { href: "/training", label: "Oceanic training" },
    { href: "/code-signing", label: "Code signing" },
    { href: "/privacy", label: "Privacy" },
];

onMount(async () => {
    site.setTitle("Homepage");
    // Someone who has asked their system for less motion gets the video paused (they can still press play)
    try {
        if (window.matchMedia("(prefers-reduced-motion: reduce)").matches && heroVideo) { heroVideo.pause(); heroPlaying = false; }
    } catch (e) { /* ignore */ }
    document.body.style.opacity = "0%";

    // Called 300ms after mount
    setTimeout(() => {
        // Do this to hide the messy content organisation that happens at first load
        document.body.style.animation = "0.2s ease-in 0s fadein";
        document.body.style.opacity = "100%";

        //
        document.getElementById("logo")!.style.animation = "0.8s linear 0s translateFromLeft normal";
        document.getElementById("logo-text")!.style.animation = "0.8s linear 0s translateFromRight normal";
        document.getElementById("learn-more")!.style.animation = "0.8s ease-in 0s fadein";
	},300);
    
});
  
</script>

<div class="h-screen scrollbar-none overflow-x-hidden" id="topDiv">
    <div class="flex flex-col items-center justify-center h-full backdrop-filter" style="background-image: url(/banner.jpg); overflow-hidden;">
        <div class="flex flex-col items-center justify-center h-1/3">
        </div>
        <div class="flex flex-col items-center justify-start h-1/3">
            <img src="/vNAAATS.png" class="overflow-hidden select-none" id="logo" width="80%" alt="Logo">
            <div class="text-white font-bitmap text-3xl pt-4 select-none" id="logo-text">
                The Virtual North Atlantic Advanced Air Traffic System
            </div>
            <div class="text-white font-bitmap text-2xl pt-2 select-none opacity-90" id="version-credit">
                Version 2 &ndash; Modernised by Gary Thomas
            </div>
            <div class="flex flex-row justify-center mt-10">
                <a href="/windows" class="border-2 rounded px-4 py-2 hover:cursor-pointer scale-100 bg-blue-5 bg-opacity-100 transition duration-500 
                hover:scale-110 hover:bg-opacity-100 font-bitmap font-normal tracking-wider text-lg">
                    FDD for Windows
                </a>
                <a href="/access" class="border-2 rounded px-4 py-2 ml-4 hover:cursor-pointer scale-100 bg-blue-5 bg-opacity-0 transition duration-500 
                hover:scale-110 hover:bg-opacity-100 font-bitmap font-normal tracking-wider text-lg">
                    Get access
                </a>
                <a href="/download" class="border-2 rounded px-4 py-2 ml-4 hover:cursor-pointer scale-100 bg-blue-5 bg-opacity-0 transition duration-500 
                hover:scale-110 hover:bg-opacity-100 font-bitmap font-normal tracking-wider text-lg">
                    Downloads
                </a>
                <a href="/fdd" class="border-2 rounded px-4 py-2 ml-4 hover:cursor-pointer scale-100 bg-blue-5 bg-opacity-0 transition duration-500 
                hover:scale-110 hover:bg-opacity-100 font-bitmap font-normal tracking-wider text-lg">
                    Use in a browser
                </a>
                <div class="relative ml-4">
                    <button type="button" aria-haspopup="true" aria-expanded={pagesOpen} on:click={() => (pagesOpen = !pagesOpen)} on:keydown={(e) => { if (e.key === "Escape") pagesOpen = false; }}
                    class="border-2 rounded px-4 py-2 hover:cursor-pointer scale-100 bg-blue-5 bg-opacity-0 transition duration-500 hover:scale-110 hover:bg-opacity-100 font-bitmap font-normal tracking-wider text-lg">
                        Pages &#9662;
                    </button>
                    {#if pagesOpen}
                        <div class="absolute right-0 mt-2 w-56 z-50 text-left bg-blue-1 border-2 rounded shadow-5xl font-bitmap text-lg">
                            {#each pages as pg}
                                <a href={pg.href} class="block px-4 py-2 hover:bg-blue-5">{pg.label}</a>
                            {/each}
                        </div>
                    {/if}
                </div>
            </div>
        </div>
        <div class="flex flex-col justify-end items-center h-1/3 font-bitmap font-normal text-2xl">
            <div class="pt-10 select-none" id="learn-more">
                Learn more
            </div>
            <svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" 
            class="mb-8 fill-current opacity-10 text-white animate-pulse scale-110 select-none" version="1.1" width="75" height="75" viewBox="0 0 24 24">
            <path d="M7,10L12,15L17,10H7Z" />
        </svg>
        </div>
    </div>
    <div class="flex flex-col items-center bg-blue-0 px-6 py-14 font-web" id="see-it">
        <div class="text-center 2xl:text-5xl xl:text-4xl text-3xl font-bitmap font-semibold tracking-wider select-none">
            See it in action
        </div>
        <div class="mt-3 mb-8 text-center text-xl opacity-80 max-w-3xl">FDD for Windows: the One Fits All Program. Flight strips, natTrak, CPDLC, VACS and audio in a single window.</div>
        <div class="relative w-full max-w-5xl border-2 border-blue-10 rounded shadow-5xl overflow-hidden bg-blue-1">
            <video bind:this={heroVideo} src="/vNAAATS-hero.mp4" poster="/vNAAATS-hero-poster.jpg" class="block w-full" style="aspect-ratio: 16 / 9;" autoplay muted loop playsinline preload="metadata" aria-label="A short tour of FDD for Windows"></video>
            <button type="button" on:click={toggleHero} class="absolute right-3 bottom-3 px-3 py-1 text-sm font-bold rounded bg-blue-1 bg-opacity-80 border border-blue-10 hover:bg-blue-5" aria-label={heroPlaying ? "Pause the video" : "Play the video"}>
                {heroPlaying ? "Pause" : "Play"}
            </button>
        </div>
        <div class="flex flex-row flex-wrap justify-center gap-4 mt-8 font-bitmap tracking-wider text-lg">
            <a href="/access" class="border-2 rounded px-5 py-2 bg-blue-5 transition duration-500 hover:scale-105">Get access</a>
            <a href="/windows" class="border-2 rounded px-5 py-2 transition duration-500 hover:scale-105 hover:bg-blue-5">Download for Windows</a>
            <a href="/videos" class="border-2 rounded px-5 py-2 transition duration-500 hover:scale-105 hover:bg-blue-5">More videos</a>
        </div>
    </div>
    <div class="flex flex-col h-1/2 justify-evenly overflow-hidden bg-blue-1" >
        <div class="px-12 2xl:pt-8 xl:pt-14 2xl:pb-2 xl:pb-0 text-center 2xl:text-5xl xl:text-4xl font-bitmap font-semibold tracking-wider select-none">
            Revolutionising North Atlantic ATC on the VATSIM Network.
        </div>
        <div class="shadow-5xl p-6 mx-10 2xl:mt-18 xl:mt-6 2xl:mb-10 xl:mb-16" style="background-color: #152C48; border-color: #152C48;">
            <div class="px-12 pt-4 text-center text-3xl font-web font-bold select-none">
                Proudly supported by:
            </div>
            <div class="flex flex-row justify-evenly">
                <div class="flex flex-row justify-center w-1/4">
                    <a href="https://vatsim.net/" class="flex items-center overflow-hidden scale-70 object-contain transition duration-500 hover:scale-80">
                        <img src="/VATSIM_Logo_No_Tagline.png" alt="VATSIM">
                    </a>
                </div>
                <div class="flex flex-row justify-center w-1/4">
                    <a href="https://ganderoceanic.net/" class="flex items-center overflow-hidden scale-70 object-contain transition duration-500 hover:scale-80">
                        <img src="/ZQO_BANNER_TRANSPARENTWHITE.png"  alt="ZQO">
                    </a>
                </div>
                <div class="flex flex-row justify-center w-1/4">
                    <a href="https://vatcan.ca/" class="flex items-top overflow-hidden scale-70 object-contain transition duration-500 hover:scale-80">
                        <img src="/vatcanlogowhite.png" alt="VATCAN">
                    </a>
                </div>
                <div class="flex flex-row justify-end w-1/4">
                    <a href="https://vatsim.uk/" class="flex items-center overflow-hidden scale-70 object-contain transition duration-500 hover:scale-80">
                        <img src="/vatsimuk_whiteblue.png" alt="VATUK">
                    </a>
                </div>
            </div>
        </div>
    </div>
    <div class="flex flex-col justify-start shadow-5xl pb-6" style="background-color: #1A3351; min-height: 50%;">
        <div class="flex flex-row justify-center w-full">
            <div class="py-8 px-12 text-center 2xl:text-5xl xl:text-4xl font-bitmap font-semibold tracking-widest select-none">
                Features &amp; Toolsets
            </div>
        </div>
        <div class="grid grid-cols-1 md:grid-cols-2 xl:grid-cols-3 gap-6 w-11/12 max-w-7xl mx-auto py-4 font-web font-bold">
            <a href="https://github.com/garythms/vatsim-NAAATS-v2" target="_blank" rel="noopener" class="flex justify-center items-stretch scale-100 transition duration-500 hover:scale-105">
                <div class="flex flex-col justify-center items-center w-full p-5 min-h-56 text-center shadow-5xl bg-blue-1 border-2 border-blue-1 rounded">
                    <div class="2xl:text-3xl xl:text-2xl">Oceanic EuroScope Plugin</div>
                    <svg xmlns="http://www.w3.org/2000/svg" version="1.1" width="90" height="90" viewBox="0 0 24 24" class="py-4 fill-current text-white" aria-hidden="true"><path d="M19.07,4.93L17.66,6.34C19.1,7.79 20,9.79 20,12A8,8 0 0,1 12,20A8,8 0 0,1 4,12C4,7.92 7.05,4.56 11,4.07V6.09C8.16,6.57 6,9.03 6,12A6,6 0 0,0 12,18A6,6 0 0,0 18,12C18,10.34 17.33,8.84 16.24,7.76L14.83,9.17C15.55,9.9 16,10.9 16,12A4,4 0 0,1 12,16A4,4 0 0,1 8,12C8,10.14 9.28,8.59 11,8.14V10.28C10.4,10.63 10,11.26 10,12A2,2 0 0,0 12,14A2,2 0 0,0 14,12C14,11.26 13.6,10.62 13,10.28V2H12A10,10 0 0,0 2,12A10,10 0 0,0 12,22A10,10 0 0,0 22,12C22,9.24 20.88,6.74 19.07,4.93Z" /></svg>
                    <div class="font-normal text-base opacity-80">Oceanic clearances, track and route tools inside EuroScope.</div>
                </div>
            </a>
            <a href="/fdd" class="flex justify-center items-stretch scale-100 transition duration-500 hover:scale-105">
                <div class="flex flex-col justify-center items-center w-full p-5 min-h-56 text-center shadow-5xl bg-blue-1 border-2 border-blue-1 rounded">
                    <div class="2xl:text-3xl xl:text-2xl">Flight Data Display</div>
                    <svg xmlns="http://www.w3.org/2000/svg" version="1.1" width="90" height="90" viewBox="0 0 24 24" class="py-4 fill-current text-white" aria-hidden="true"><path d="M19 12V13.5C21.21 13.5 23 15.29 23 17.5C23 18.32 22.76 19.08 22.33 19.71L21.24 18.62C21.41 18.28 21.5 17.9 21.5 17.5C21.5 16.12 20.38 15 19 15V16.5L16.75 14.25L19 12M19 23V21.5C16.79 21.5 15 19.71 15 17.5C15 16.68 15.25 15.92 15.67 15.29L16.76 16.38C16.59 16.72 16.5 17.1 16.5 17.5C16.5 18.88 17.62 20 19 20V18.5L21.25 20.75L19 23M4 3H18C19.11 3 20 3.9 20 5V11L19.5 11C17.66 11 16 11.77 14.81 13H12V17H13L13 17.5C13 18 13.06 18.5 13.17 19H4C2.9 19 2 18.11 2 17V5C2 3.9 2.9 3 4 3M4 7V11H10V7H4M12 7V11H18V7H12M4 13V17H10V13H4Z" /></svg>
                    <div class="font-normal text-base opacity-80">Live strips for every flight. Works in any browser, such as an iPad. On Windows, use FDD for Windows.</div>
                </div>
            </a>
            <a href="/windows" class="flex justify-center items-stretch scale-100 transition duration-500 hover:scale-105">
                <div class="flex flex-col justify-center items-center w-full p-5 min-h-56 text-center shadow-5xl bg-blue-1 border-2 border-blue-1 rounded">
                    <div class="2xl:text-3xl xl:text-2xl">FDD for Windows</div>
                    <svg xmlns="http://www.w3.org/2000/svg" version="1.1" width="90" height="90" viewBox="0 0 24 24" class="py-4 fill-current text-white" aria-hidden="true"><path d="M3 3h8v8H3zM13 3h8v8h-8zM3 13h8v8H3zM13 13h8v8h-8z" /></svg>
                    <div class="font-normal text-base opacity-80">The recommended way on Windows: one installer, and plugin updates and setup look after themselves.</div>
                </div>
            </a>
            <a href="/guide" class="flex justify-center items-stretch scale-100 transition duration-500 hover:scale-105">
                <div class="flex flex-col justify-center items-center w-full p-5 min-h-56 text-center shadow-5xl bg-blue-1 border-2 border-blue-1 rounded">
                    <div class="2xl:text-3xl xl:text-2xl">VACS Call Alerts</div>
                    <svg xmlns="http://www.w3.org/2000/svg" width="90" height="90" viewBox="0 0 24 24" fill="none" stroke="#7fe0a2" stroke-width="1.6" class="my-4" aria-hidden="true"><path d="M4 15v-3a8 8 0 0 1 16 0v3" /><rect x="3" y="14" width="4" height="6" rx="1.5" /><rect x="17" y="14" width="4" height="6" rx="1.5" /></svg>
                    <div class="font-normal text-base opacity-80">A headphones button that flashes when VACS is calling you.</div>
                </div>
            </a>
            <a href="/manual" class="flex justify-center items-stretch scale-100 transition duration-500 hover:scale-105">
                <div class="flex flex-col justify-center items-center w-full p-5 min-h-56 text-center shadow-5xl bg-blue-1 border-2 border-blue-1 rounded">
                    <div class="2xl:text-3xl xl:text-2xl">Dedicated Network &amp; API</div>
                    <svg xmlns="http://www.w3.org/2000/svg" version="1.1" width="90" height="90" viewBox="0 0 24 24" class="py-4 fill-current text-white" aria-hidden="true"><path d="M10,2C8.89,2 8,2.89 8,4V7C8,8.11 8.89,9 10,9H11V11H2V13H6V15H5C3.89,15 3,15.89 3,17V20C3,21.11 3.89,22 5,22H9C10.11,22 11,21.11 11,20V17C11,15.89 10.11,15 9,15H8V13H16V15H15C13.89,15 13,15.89 13,17V20C13,21.11 13.89,22 15,22H19C20.11,22 21,21.11 21,20V17C21,15.89 20.11,15 19,15H18V13H22V11H13V9H14C15.11,9 16,8.11 16,7V4C16,2.89 15.11,2 14,2H10M10,4H14V7H10V4M5,17H9V20H5V17M15,17H19V20H15V17Z" /></svg>
                    <div class="font-normal text-base opacity-80">Our own server keeps every controller on the same data.</div>
                </div>
            </a>
            <a href="/training" class="flex justify-center items-stretch scale-100 transition duration-500 hover:scale-105">
                <div class="flex flex-col justify-center items-center w-full p-5 min-h-56 text-center shadow-5xl bg-blue-1 border-2 border-blue-1 rounded">
                    <div class="2xl:text-3xl xl:text-2xl">Oceanic Training</div>
                    <svg xmlns="http://www.w3.org/2000/svg" version="1.1" width="90" height="90" viewBox="0 0 24 24" class="py-4 fill-current text-white" aria-hidden="true"><path d="M8 5v14l11-7z" /></svg>
                    <div class="font-normal text-base opacity-80">Trainee video, how-to videos and a checklist for your first session.</div>
                </div>
            </a>
            <a href="/manual" class="flex justify-center items-stretch scale-100 transition duration-500 hover:scale-105">
                <div class="flex flex-col justify-center items-center w-full p-5 min-h-56 text-center shadow-5xl bg-blue-1 border-2 border-blue-1 rounded">
                    <div class="2xl:text-3xl xl:text-2xl">Dedicated CPDLC Interaction</div>
                    <svg xmlns="http://www.w3.org/2000/svg" version="1.1" width="90" height="90" viewBox="0 0 24 24" class="py-4 fill-current text-white" aria-hidden="true"><path d="M20,2H4A2,2 0 0,0 2,4V22L6,18H20A2,2 0 0,0 22,16V4A2,2 0 0,0 20,2M6,9H18V11H6M14,14H6V12H14M18,8H6V6H18" /></svg>
                    <div class="font-normal text-base opacity-80">Message pilots from the FDD. Auto RCL reminders ask first.</div>
                </div>
            </a>
        </div>
    </div>
    <div class="flex flex-row h-1/14 justify-evenly items-center font-web font-semibold overflow-hidden bg-blue-0" >
        <div class="tracking-wide text-center">
            Copyright © Andrew Ogden 2020-2021 (original vNAAATS). Version 2 © Gary Thomas 2026. FOR FLIGHT SIMULATION ONLY. NOT TO BE USED FOR REAL WORLD NAVIGATION.
            <br />Independent community project, not run or endorsed by VATSIM. <a href="/privacy" class="underline">Privacy and data handling</a>. <a href="/code-signing" class="underline">Code signing policy</a>.
            <br />Learn more about the VATSIM network <a href="https://vatsim.net" class="underline">here</a>.
        </div>
    </div>
</div>