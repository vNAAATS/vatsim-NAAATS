/*
 * Site variables and functions (dynamic)
 * Created by Andrew Ogden
 */

/*
 *  Imports
 */

/*
 *   Properties
 */
export let siteSection: number = 0;

/*
 *  Functions
 */
// We do this so that we can hide the index page layout
export function setSiteSection(arg: number) : void {
  siteSection = arg;
}
// Set the tab bar title
export function setTitle(arg: string) : void {
  document.title = arg + " - vNAAATS";
}
