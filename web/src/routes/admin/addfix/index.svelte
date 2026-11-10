<script lang="ts">
    import TextInput from "../../../props/fdd/text_input.svelte";
    import Button from "../../../props/fdd/button.svelte";
    import * as site from "../../..//typescript/site";
    import { onMount } from "svelte";

    let name: string = "";
    let latitude: string = "";
    let longitude: string = "";
    let token: string = "";
    let info: string[];
    $: info = [];
    onMount(async () => {
        document.body.style.opacity = "100%";
        site.setTitle("Add named fix");
        site.setSiteSection(1);
    });

    async function submitBtn() {
        // Validation
        info = [];
        if (name == "" || latitude == "" || longitude == "" || token == "") {
            info.push("Please fill out all of the input fields");
            info = info;
            return;
        }

        if (name.length != 5) {
            info.push("Name must be 5 characters long");
        } else {
            name = name.toUpperCase();
        }
        
        let lat: number = parseFloat(latitude);
        if (isNaN(lat)) {
            info.push("Latitude must be a floating point number");
        }

        let lon: number = parseFloat(longitude);
        if (isNaN(lon)) {
            info.push("Longitude must be a floating point number.");
        }
        
        if (info.length > 0) {
            info = info;
            return;
        }

        // Construct link
        let postLink: string = ((import.meta as any).env?.VITE_API_BASE ?? "") + "/PostNamedFix?";
        postLink += "name=" + name;
        postLink += "&lat=" + lat;
        postLink += "&lon=" + lon;
        postLink += "&code=" + encodeURIComponent(token);
        console.log(postLink)
         // Post data
        await fetch(postLink).then(function (response: Response) {
            if (response.status == 401) { // Incorrect token
                info.push("Incorrect token.");
                info = info;
                return;
            } else if (response.status != 200) { // Catch any other error
                info.push("An error occurred on the remote server.");
                info.push("Response dump: " + response.text());
                info = info;
                return;
            }
            // If we got here then it worked fine
            response.text().then(function (res: string) {
                    info.push("Fix added successfully.")
                    info.push("Name: " + name);
                    info.push("Latitude: " + lat.toString());
                    info.push("Longitude: " + lon.toString());
                    info = info;
            });
        });
    }
</script>

<div class="flex flex-col items-start h-screen w-screen">
    <div class="mx-1 my-2">
        <TextInput label={"Fix name:"} identifier={"fix_name"} width={"w-36"} bind:value={name} charWidth={5}/>
    </div>
    <div class="mx-1 my-2">
        <TextInput label={"Latitude:"} identifier={"fix_latitude"} width={"w-36"} bind:value={latitude} />
    </div>
    <div class="mx-1 my-2">
        <TextInput label={"Longitude:"} identifier={"fix_longitude"} width={"w-36"} bind:value={longitude}/>
    </div>
    <div class="mx-1 my-2">
        <TextInput label={"Token:"} identifier={"fix_authtoken"} width={"w-48"} inputType={2}  bind:value={token} charWidth={25} />
    </div>
    <div class="mx-1 my-2 scale-80" on:click={submitBtn}>
        <Button label="Submit"/>
    </div>
    <div class="mx-1 my-3 scale-90">
        {#each info as item}
            {item}
            <br>
        {/each}
    </div>
    
</div>