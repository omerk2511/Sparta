<template>
    <v-main>
        <v-container class="py-8 px-6" fluid>
            <v-row>
                <v-col cols="12">
                    <v-card>
                        <v-row align="start">
                            <v-col class="shrink">
                                <v-img src="../assets/me.png" max-width="110" class="ml-3"></v-img>
                            </v-col>
                            <v-col>
                                <v-card-title class="pa-1 pb-4">About</v-card-title>
                                <v-card-subtitle class="pa-1">Sparta is my final 5 unit Cyber project</v-card-subtitle>
                                <v-card-subtitle class="pa-1">It is a hypervisor-based endpoint protection platform which detects severe threats at the lowest level possible</v-card-subtitle>
                            </v-col>
                        </v-row>

                        <v-list>
                            <template v-for="[icon, text, link] in links">
                                <v-list-item link target="_blank" :key="link" :href="link">
                                    <v-list-item-icon>
                                        <v-icon color="secondary">{{ icon }}</v-icon>
                                    </v-list-item-icon>

                                    <v-list-item-content>
                                        <v-list-item-title>{{ text }}</v-list-item-title>
                                    </v-list-item-content>
                                </v-list-item>
                            </template>
                        </v-list>
                    </v-card>
                </v-col>
            </v-row>
        </v-container>
    </v-main>
</template>

<script>
export default {
    name: 'About',

    title: 'Sparta - About',

    data: () => ({
        links: [
            ['mdi-github', 'Star the project on Github', 'https://github.com/omerk2511/Sparta'],
            ['mdi-twitter', 'Follow me on Twitter', 'https://twitter.com/omerk2511'],
            ['mdi-linkedin', 'Connect to me on LinkedIn', 'https://www.linkedin.com/in/omer-katz'],
        ]
    })
};
</script>
